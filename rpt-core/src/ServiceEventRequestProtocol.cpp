#include <RpT-Core/ServiceEventRequestProtocol.hpp>

#include <algorithm>
#include <cassert>


namespace RpT::Core {


ServiceEventRequestProtocol::ServiceRequestCommandParser::ServiceRequestCommandParser(
        const std::string_view sr_command) : Utils::TextProtocolParser { sr_command, 2 } {}

bool ServiceEventRequestProtocol::ServiceRequestCommandParser::isValidRequest() const {
    return getParsedWord(0) == REQUEST_PREFIX;
}

std::string_view ServiceEventRequestProtocol::ServiceRequestCommandParser::intendedServiceName() const {
    return getParsedWord(1);
}

std::string_view ServiceEventRequestProtocol::ServiceRequestCommandParser::commandData() const {
    return unparsedWords();
}


std::vector<std::string_view> ServiceEventRequestProtocol::getWordsFor(std::string_view sr_command) {
    // Begin and end constant iterators for SR command string
    const auto cmd_begin { sr_command.cbegin() };
    const auto cmd_end { sr_command.cend() };

    // Starts without any word parsed yet
    std::vector<std::string_view> parsed_words;

    // Starts string parsing from the first word
    auto word_begin { cmd_begin };
    auto word_end { std::find(cmd_begin, cmd_end, ' ') };

    // While entire string hasn't be parsed (while word begin isn't at string end)
    while (word_begin != word_end) {
        // Calculate index for word begin, and next word size
        const auto word_begin_i { word_begin - cmd_begin };
        const auto word_length { word_end - word_begin };

        parsed_words.push_back(sr_command.substr(word_begin_i, word_length )); // Add currently parsed word to words

        word_begin = word_end; // Is the string end reached ?

        if (word_end != cmd_end) { // Is the string end reached ?
            word_begin++; // Then, move word begin after previously found space
            word_end = std::find(word_begin, cmd_end, ' '); // And find next word end starting from next word begin
        }
    }

    return parsed_words;
}

ServiceEventRequestProtocol::ServiceEventRequestProtocol(
        const std::initializer_list<std::reference_wrapper<Service>>& services,
        Utils::LoggingContext& logging_context) :

        logger_ { "SER-Protocol", logging_context } {

    // Each given service reference must be registered as running service
    for (const auto service_ref : services) {
        const std::string_view service_name { service_ref.get().name() };

        if (isRegistered(service_name)) // Service name must be unique among running services
            throw ServiceNameAlreadyRegistered { service_ref.get().name() };

        const auto service_registration_result { running_services_.insert({ service_name, service_ref }) };
        // Must be sure that service has been successfully registered, this is why insertion result is saved
        assert(service_registration_result.second);

        logger_.debug("Registered service {}.", service_name);
    }
}

bool ServiceEventRequestProtocol::isRegistered(const std::string_view service) const {
    return running_services_.count(service) == 1; // Returns if service name is present among running services
}

Utils::HandlingResult ServiceEventRequestProtocol::handleServiceRequest(uint64_t actor,
                                                                        std::string_view service_request) {

    logger_.trace("Handling SR command from \"{}\": {}", actor, service_request);

    std::string_view intended_service_name;
    std::string_view command_data;
    try { // Tries to parse received SR command
        const ServiceRequestCommandParser sr_command_parser { service_request }; // Parsing

        if (!sr_command_parser.isValidRequest()) // Checks for SER command prefix, must be REQUEST for a SR command
            throw InvalidRequestFormat { service_request, "Expected SER command prefix \"REQUEST\" for SR command" };

        // Set given parameters to corresponding parsed (or not) arguments
        intended_service_name = sr_command_parser.intendedServiceName();
        command_data = sr_command_parser.commandData();
    } catch (const Utils::NotEnoughWords& err) { // Catches error if SER command format is invalid
        throw InvalidRequestFormat { service_request, "Expected SER command prefix and request service name" };
    }

    assert(!intended_service_name.empty()); // Service name must be initialized if try statement passed successfully
    Service& intended_service { running_services_.at(intended_service_name).get() };

    logger_.trace("SR command successfully parsed, handled by service: {}", intended_service_name);

    // Try to handle SR command, catching errors occurring inside handlers
    try {
        // Handles SR command and retrieves result
        return intended_service.handleRequestCommand(actor, std::vector<std::string_view> { command_data });
    } catch (const std::exception& err) {
        logger_.error("Service \"{}\" failed to handle command: {}" , intended_service_name, err.what());

        // Retrieves error result with given caught message
        return Utils::HandlingResult { err.what() };
    }
}


std::optional<std::string> ServiceEventRequestProtocol::pollServiceEvent() {
    std::optional<std::string> next_event; // Event to poll is first uninitialized

    Service* newest_event_emitter { nullptr }; // Service with event which has the lowest ID
    std::size_t lowest_event_id;
    for (auto registered_service : running_services_) {
        Service& service { registered_service.second.get() };

        const std::optional<std::size_t> next_event_id { service.checkEvent() };

        if (next_event_id.has_value()) { // Skip service if has an empty events queue
            logger_.trace("Service {} last event ID: {}", service.name(), *next_event_id);

            // If there isn't any event to poll or current was triggered first...
            if (!newest_event_emitter || next_event_id < lowest_event_id) {
                // ...then set new event emitter, and update lowest ID
                newest_event_emitter = &service;
                lowest_event_id = *next_event_id;
            }
        } else {
            logger_.trace("Service {} hasn't any event.", service.name());
        }
    }

    if (newest_event_emitter) { // If there is any emitted event, format it and move it into polled event
        const std::string service_name_copy { newest_event_emitter->name() }; // Required for concatenation

        next_event = std::string { EVENT_PREFIX } + ' ' + service_name_copy + ' ' + newest_event_emitter->pollEvent();

        logger_.trace("Polled event from service {}: {}", newest_event_emitter->name(), *next_event);
    } else {
        logger_.trace("No event to retrieve.");
    }

    return next_event;
}


}