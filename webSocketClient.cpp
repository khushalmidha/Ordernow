/**
 * @file webSocketClient.cpp
 * @brief Implementation of the WebSocket client for interacting with a WebSocket server.
 */

#include "webSocketClient.h"
#include "deriapi.h"
#include <fmt/core.h> // Use fmt for formatted output
#include <iostream>
#include <thread>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

/**
 * @brief Constructs a new WebSocket client.
 *
 * Initializes the WebSocket endpoint, disables logging, sets up TLS, and configures event handlers.
 */
webSocketClient::webSocketClient() 
    : m_connected(false), 
      m_authRequestCallback(nullptr), 
      m_authenticated(false), 
      m_waitingForResponse(false) {
    // Disable logging for cleaner output
    m_endpoint.clear_access_channels(websocketpp::log::alevel::all);
    m_endpoint.clear_error_channels(websocketpp::log::elevel::all);

    // Initialize ASIO and start perpetual loop
    m_endpoint.init_asio();
    m_endpoint.start_perpetual();
    
    // Set up TLS handler
    m_endpoint.set_tls_init_handler([this](websocketpp::connection_hdl) {
        return websocketpp::lib::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::tlsv12);
    });

    // Set up event handlers
    m_endpoint.set_open_handler([this](auto hdl) { this->on_open(&m_endpoint, hdl); });
    m_endpoint.set_fail_handler([this](auto hdl) { this->on_fail(&m_endpoint, hdl); });
    m_endpoint.set_close_handler([this](auto hdl) { this->on_close(&m_endpoint, hdl); });
    m_endpoint.set_message_handler([this](auto hdl, auto msg) { this->on_message(&m_endpoint, hdl, msg); });
}

/**
 * @brief Destructor for the WebSocket client.
 *
 * Stops the perpetual loop and closes the connection if it is still open.
 */
webSocketClient::~webSocketClient() {
    m_endpoint.stop_perpetual();
    if (m_connected) {
        close();
    }
}

/**
 * @brief Sets the authentication request callback.
 *
 * @param callback A function to be called when authentication is requested.
 */
void webSocketClient::setAuthRequestCallback(std::function<void()> callback) {
    m_authRequestCallback = callback;
}

/**
 * @brief Sends a message through the WebSocket connection.
 *
 * @param message The message to send.
 */
void webSocketClient::send(const std::string& message) {
    websocketpp::lib::error_code ec;
    m_endpoint.send(m_hdl, message, websocketpp::frame::opcode::text, ec);
    if (ec) {
        fmt::print(stderr, "Send error: {}\n", ec.message());
    }
}

/**
 * @brief Connects to a WebSocket server.
 *
 * @param uri The URI of the WebSocket server to connect to.
 */
void webSocketClient::connect(const std::string& uri) {
    websocketpp::lib::error_code ec;
    client::connection_ptr con = m_endpoint.get_connection(uri, ec);
    if (ec) {
        fmt::print(stderr, "Connection error: {}\n", ec.message());
        return;
    }

    m_endpoint.connect(con);
    m_eventLoopThread = std::thread([this]() { m_endpoint.run(); });
}

/**
 * @brief Closes the WebSocket connection.
 */
void webSocketClient::close() {
    if (m_connected) {
        websocketpp::lib::error_code ec;
        m_endpoint.close(m_hdl, websocketpp::close::status::normal, "Closing Connection", ec);
        if (ec) {
            fmt::print(stderr, "Close error: {}\n", ec.message());
            return;
        }
        m_connected = false;
    }
    m_endpoint.stop_perpetual();
    m_endpoint.stop();

    if (m_eventLoopThread.joinable()) {
        m_eventLoopThread.join();
    }
}

/**
 * @brief Handles the WebSocket connection open event.
 *
 * @param c Pointer to the WebSocket client.
 * @param hdl The connection handle.
 */
void webSocketClient::on_open(client* c, websocketpp::connection_hdl hdl) {
    fmt::print("Connection opened!\n");
    m_hdl = hdl;
    m_connected = true;
    if (m_authRequestCallback) {
        m_authRequestCallback();
    }
}

/**
 * @brief Handles the WebSocket connection fail event.
 *
 * @param c Pointer to the WebSocket client.
 * @param hdl The connection handle.
 */
void webSocketClient::on_fail(client* c, websocketpp::connection_hdl hdl) {
    fmt::print(stderr, "Connection failed!\n");
    m_connected = false;
}

/**
 * @brief Handles the WebSocket connection close event.
 *
 * @param c Pointer to the WebSocket client.
 * @param hdl The connection handle.
 */
void webSocketClient::on_close(client* c, websocketpp::connection_hdl hdl) {
    fmt::print("Connection closed!\n");
    m_connected = false;
}

/**
 * @brief Subscribes to a WebSocket channel.
 *
 * @param channel The name of the channel to subscribe to.
 */
void webSocketClient::subscribe(const std::string& channel) {
    std::string subscribeRequest = deriapi::subscribeToChannel(channel);
    send(subscribeRequest);
    m_subscribedChannels.insert(channel);
    m_lastData[channel] = ""; 
}

/**
 * @brief Unsubscribes from a WebSocket channel.
 *
 * @param channel The name of the channel to unsubscribe from.
 */
void webSocketClient::unsubscribe(const std::string& channel) {
    fmt::print("Unsubscribed from channel: {}\n", channel);
    std::string unsubscribeRequest = deriapi::unsubscribeFromChannel(channel);
    send(unsubscribeRequest);
    m_subscribedChannels.erase(channel);
    m_lastData.erase(channel);    
    fmt::print("Unsubscribed from channel: {}\n", channel);
}

/**
 * @brief Handles subscription messages for a specific channel.
 *
 * @param channel The name of the channel.
 * @param data The JSON data received for the channel.
 */
void webSocketClient::handleSubscriptionMessage(const std::string& channel, const nlohmann::json& data) {
    try {
        if (channel.find("ticker") != std::string::npos) {
            // Handle ticker data
            if (data.is_object()) {
                fmt::print("Ticker Update ({}): {}\n", channel, data.dump(2));
            } else if (data.is_number() || data.is_string() || data.is_boolean()) {
                fmt::print("Ticker Update ({}): {}\n", channel, data.dump(2));
            } else {
                fmt::print(stderr, "Unexpected data type for ticker channel '{}'.\n", channel);
            }
        } else if (channel.find("trades") != std::string::npos) {
            // Handle trades data
            if (data.is_array()) {
                fmt::print("Trade Update ({}): {}\n", channel, data.dump(2));
            } else {
                fmt::print(stderr, "Unexpected data type for trades channel '{}'.\n", channel);
            }
        } else if (channel.find("book") != std::string::npos) {
            // Handle order book data
            if (data.is_object()) {
                fmt::print("Order Book Update ({}): {}\n", channel, data.dump(2));
            } else {
                fmt::print(stderr, "Unexpected data type for book channel '{}'.\n", channel);
            }
        } else {
            // Handle other channels
            fmt::print("Update ({}): {}\n", channel, data.dump(2));
        }
    } catch (const nlohmann::json::exception& e) {
        fmt::print(stderr, "JSON Parsing Error in channel '{}': {}\n", channel, e.what());
    }
}

/**
 * @brief Handles authentication success messages.
 *
 * @param result The JSON result containing authentication details.
 */
void webSocketClient::on_message_auth(nlohmann::json result) {
    fmt::print("Authentication successful!\n");
}

/**
 * @brief Handles account summary messages.
 *
 * @param result The JSON result containing account summary details.
 */
void webSocketClient::on_message_summary(nlohmann::json result) {
    fmt::print("\nAccount Summary:\n");
    fmt::print("Balance: {}\n", result.value("balance", 0.0));
    fmt::print("Currency: {}\n", result.value("currency", "N/A"));
    fmt::print("Equity: {}\n", result.value("equity", 0.0));
    fmt::print("Initial Margin: {}\n", result.value("initial_margin", 0.0));
    fmt::print("Maintenance Margin: {}\n", result.value("maintenance_margin", 0.0));
    fmt::print("Available Funds: {}\n", result.value("available_funds", 0.0));
    fmt::print("Margin Balance: {}\n", result.value("margin_balance", 0.0));
}

/**
 * @brief Handles buy order success messages.
 *
 * @param order The JSON result containing buy order details.
 */
void webSocketClient::on_message_buy(nlohmann::json order) {
    fmt::print("Buy Order Placed Successfully!\n");
    fmt::print("Order ID: {}\n", order.value("order_id", "N/A"));
    fmt::print("Instrument: {}\n", order.value("instrument_name", "N/A"));
    fmt::print("Direction: {}\n", order.value("direction", "N/A"));
    fmt::print("Amount: {}\n", order.value("amount", 0.0));
    fmt::print("Price: {}\n", order.value("price", 0.0));
    fmt::print("Order Type: {}\n", order.value("order_type", "N/A"));
    fmt::print("Order State: {}\n", order.value("order_state", "N/A"));
    fmt::print("Filled Amount: {}\n", order.value("filled_amount", 0.0));
    fmt::print("Average Price: {}\n", order.value("average_price", 0.0));
    fmt::print("Creation Timestamp: {}\n", order.value("creation_timestamp", 0));
    fmt::print("Last Update Timestamp: {}\n", order.value("last_update_timestamp", 0));
}

/**
 * @brief Handles order cancellation success messages.
 *
 * @param result The JSON result containing cancellation details.
 */
void webSocketClient::on_message_cancel(nlohmann::json result) {
    fmt::print("Canceled Order Successfully!\n");
    fmt::print("Order ID: {}\n", result.value("order_id", "N/A"));
    fmt::print("Time in Force: {}\n", result.value("time_in_force", "N/A"));
    fmt::print("Order Type: {}\n", result.value("order_type", "N/A"));
}

/**
 * @brief Handles order book update messages.
 *
 * @param result The JSON result containing order book details.
 */
void webSocketClient::on_message_orderBook(nlohmann::json result) {
    try {
        nlohmann::json orderBook = result;

        // Print order book details
        fmt::print("\nOrder Book Details:\n");
        fmt::print("Instrument: {}\n", orderBook.value("instrument_name", "N/A")); // String
        fmt::print("Timestamp: {}\n", orderBook.value("timestamp", 0));           // Number
        fmt::print("Last Price: {}\n", orderBook.value("last_price", 0.0));       // Number
        fmt::print("Best Bid Price: {}\n", orderBook.value("best_bid_price", 0.0)); // Number
        fmt::print("Best Bid Amount: {}\n", orderBook.value("best_bid_amount", 0.0)); // Number
        fmt::print("Best Ask Price: {}\n", orderBook.value("best_ask_price", 0.0)); // Number
        fmt::print("Best Ask Amount: {}\n", orderBook.value("best_ask_amount", 0.0)); // Number
        fmt::print("Mark Price: {}\n", orderBook.value("mark_price", 0.0));       // Number
        fmt::print("Open Interest: {}\n", orderBook.value("open_interest", 0.0)); // Number
        fmt::print("Funding Rate (8h): {}\n", orderBook.value("funding_8h", 0.0)); // Number

        // Handle bids
        fmt::print("\nBids:\n");
        if (orderBook.contains("bids") && orderBook["bids"].is_array()) {
            for (const auto& bid : orderBook["bids"]) {
                if (bid.is_array() && bid.size() >= 2) {
                    fmt::print("Price: {}, Amount: {}\n", bid[0].get<double>(), bid[1].get<double>());
                }
            }
        } else {
            fmt::print("No bids found.\n");
        }

        // Handle asks
        fmt::print("\nAsks:\n");
        if (orderBook.contains("asks") && orderBook["asks"].is_array()) {
            for (const auto& ask : orderBook["asks"]) {
                if (ask.is_array() && ask.size() >= 2) {
                    fmt::print("Price: {}, Amount: {}\n", ask[0].get<double>(), ask[1].get<double>());
                }
            }
        } else {
            fmt::print("No asks found.\n");
        }
    } catch (const nlohmann::json::exception& e) {
        fmt::print(stderr, "JSON Parsing Error: {}\n", e.what());
    }
}

/**
 * @brief Handles order modification success messages.
 *
 * @param result The JSON result containing modification details.
 */
void webSocketClient::on_message_modify(nlohmann::json result) {
    fmt::print("\nOrder Modified Successfully!\n");
    fmt::print("Order ID: {}\n", result.value("order_id", "N/A"));
    fmt::print("New Amount: {}\n", result.value("amount", 0.0));
    fmt::print("New Price: {}\n", result.value("price", 0.0));
    fmt::print("Order State: {}\n", result.value("order_state", "N/A"));
}

/**
 * @brief Handles position update messages.
 *
 * @param result The JSON result containing position details.
 */
void webSocketClient::on_message_positions(nlohmann::json result) {
    if (result.empty()) {
        fmt::print("No positions found.\n");
        return;
    }
    fmt::print("\nCurrent Positions:\n");
    for (const auto& position : result) {
        fmt::print("Instrument: {}\n", position.value("instrument_name", "N/A"));
        fmt::print("Size: {}\n", position.value("size", 0.0));
        fmt::print("Direction: {}\n", position.value("direction", "N/A"));
        fmt::print("Average Price: {}\n", position.value("average_price", 0.0));
        fmt::print("Mark Price: {}\n", position.value("mark_price", 0.0));
        fmt::print("Total Profit/Loss: {}\n", position.value("total_profit_loss", 0.0));
        fmt::print("Floating Profit/Loss: {}\n", position.value("floating_profit_loss", 0.0));
        fmt::print("Realized Profit/Loss: {}\n", position.value("realized_profit_loss", 0.0));
        fmt::print("Initial Margin: {}\n", position.value("initial_margin", 0.0));
        fmt::print("Maintenance Margin: {}\n", position.value("maintenance_margin", 0.0));
        fmt::print("Leverage: {}\n", position.value("leverage", 0.0));
        fmt::print("Estimated Liquidation Price: {}\n", position.value("estimated_liquidation_price", 0.0));
        fmt::print("----------------------------\n");
    }
}




/**
 * @brief Handles incoming WebSocket messages.
 *
 * @param c Pointer to the WebSocket client.
 * @param hdl The connection handle.
 * @param msg The received message.
 */
void webSocketClient::on_message(client* c, websocketpp::connection_hdl hdl, client::message_ptr msg) {
    try {
        nlohmann::json response = nlohmann::json::parse(msg->get_payload());
        if (response.contains("method") && response["method"] == "subscription") {
            if (response.contains("params") && response["params"].is_object()) {
                std::string channel;
                if (response["params"].contains("channel")) {
                    if (response["params"]["channel"].is_string()) {
                        channel = response["params"]["channel"];
                    } else if (response["params"]["channel"].is_object() && response["params"]["channel"].contains("name")) {
                        channel = response["params"]["channel"]["name"];
                    } else {
                        fmt::print(stderr, "Invalid channel format in JSON response.\n");
                        return;
                    }
                }

                if (m_lastData.find(channel) == m_lastData.end()) {
                    fmt::print("Unsubscribed successfully from channel.\n");
                    return; // Channel is unsubscribed, ignore this message
                }

                // Handle the "data" field based on its type
                if (response["params"].contains("data")) {
                    nlohmann::json data = response["params"]["data"];

                    // Check the type of "data"
                    if (data.is_object()) {
                        // Handle object data (e.g., book channel)
                        if (m_lastData[channel] != data.dump()) {
                            m_lastData[channel] = data.dump(); // Update the last data
                            handleSubscriptionMessage(channel, data); // Process the new data
                        }
                    } else if (data.is_array()) {
                        // Handle array data (e.g., trades channel)
                        if (m_lastData[channel] != data.dump()) {
                            m_lastData[channel] = data.dump(); // Update the last data
                            handleSubscriptionMessage(channel, data); // Process the new data
                        }
                    } else if (data.is_string() || data.is_number() || data.is_boolean()) {
                        // Handle primitive data (e.g., ticker channel)
                        if (m_lastData[channel] != data.dump()) {
                            m_lastData[channel] = data.dump(); // Update the last data
                            handleSubscriptionMessage(channel, data); // Process the new data
                        }
                    } else {
                        fmt::print(stderr, "Unexpected data type in channel '{}'.\n", channel);
                    }
                } else {
                    fmt::print(stderr, "No data field found in channel '{}'.\n", channel);
                }
            }
        } else if (response.contains("result")) {
            if (response["result"].contains("access_token")) {
                m_accessToken = response["result"]["access_token"];
                on_message_auth(response["result"]);
                m_authenticated = true;
            } else if (response["result"].contains("balance")) {
                on_message_summary(response["result"]);
            } else if (response["result"].contains("order")) {
                on_message_buy(response["result"]["order"]);
            } else if (response["result"].contains("order_id")) {
                on_message_cancel(response["result"]);
            } else if (response["result"].contains("bids") && response["result"].contains("asks")) {
                on_message_orderBook(response["result"]);
            } else if (response["result"].contains("order_id")) {
                on_message_modify(response["result"]);
            } else if (response["result"].is_array()) {
                on_message_positions(response["result"]);
            } 
        } else if (response.contains("error")) {
            fmt::print(stderr, "Error: {}\n", response["error"].value("message", "Unknown error"));
        }
    } catch (const nlohmann::json::exception& e) {
        fmt::print(stderr, "Error parsing JSON response: {}\n", e.what());
    }
}

/**
 * @brief Checks if the client is authenticated.
 *
 * @return True if authenticated, false otherwise.
 */
bool webSocketClient::isAuthenticated() const {
    return m_authenticated;
}

/**
 * @brief Checks if the client is waiting for a response.
 *
 * @return True if waiting for a response, false otherwise.
 */
bool webSocketClient::isWaitingForResponse() const {
    return m_waitingForResponse;
}

/**
 * @brief Gets the access token.
 *
 * @return The access token as a string.
 */
std::string webSocketClient::getAccessToken() const {
    return m_accessToken;
}