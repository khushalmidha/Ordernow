/**
 * @file webSocketClient.h
 * @brief Header file for the WebSocket client class.
 *
 * This file defines the `webSocketClient` class, which provides functionality
 * to connect to a WebSocket server, send and receive messages, and handle
 * various WebSocket events such as open, close, and message reception.
 */

#ifndef WEBSOCKETCLIENT_H
#define WEBSOCKETCLIENT_H

#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>
#include <websocketpp/common/thread.hpp>
#include <websocketpp/common/memory.hpp>
#include <nlohmann/json.hpp>
#include <fmt/core.h> // Use fmt for formatted output
#include <iostream>
#include <thread>
#include <map>
#include <set>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

typedef websocketpp::client<websocketpp::config::asio_tls_client> client;
typedef websocketpp::lib::shared_ptr<boost::asio::ssl::context> context_ptr;

/**
 * @class webSocketClient
 * @brief A WebSocket client for interacting with a WebSocket server.
 *
 * This class provides methods to connect to a WebSocket server, send and receive
 * messages, and handle events such as connection open, close, and message reception.
 * It also supports subscription to channels and authentication.
 */
class webSocketClient {
public:
    /**
     * @brief Constructs a new WebSocket client.
     */
    webSocketClient();

    /**
     * @brief Destructor for the WebSocket client.
     *
     * Ensures the WebSocket connection is properly closed and resources are cleaned up.
     */
    ~webSocketClient();

    /**
     * @brief Sets the authentication request callback.
     *
     * @param callback A function to be called when authentication is requested.
     */
    void setAuthRequestCallback(std::function<void()> callback);

    /**
     * @brief Sends a message through the WebSocket connection.
     *
     * @param message The message to send.
     */
    void send(const std::string& message);

    /**
     * @brief Connects to a WebSocket server.
     *
     * @param uri The URI of the WebSocket server to connect to.
     */
    void connect(const std::string& uri);

    /**
     * @brief Closes the WebSocket connection.
     */
    void close();

    /**
     * @brief Subscribes to a WebSocket channel.
     *
     * @param channel The name of the channel to subscribe to.
     */
    void subscribe(const std::string& channel);

    /**
     * @brief Unsubscribes from a WebSocket channel.
     *
     * @param channel The name of the channel to unsubscribe from.
     */
    void unsubscribe(const std::string& channel);

    /**
     * @brief Checks if the client is authenticated.
     *
     * @return True if authenticated, false otherwise.
     */
    bool isAuthenticated() const;

    /**
     * @brief Checks if the client is waiting for a response.
     *
     * @return True if waiting for a response, false otherwise.
     */
    bool isWaitingForResponse() const;

    /**
     * @brief Gets the access token.
     *
     * @return The access token as a string.
     */
    std::string getAccessToken() const;

private:
    /**
     * @brief Handles the WebSocket connection open event.
     *
     * @param c Pointer to the WebSocket client.
     * @param hdl The connection handle.
     */
    void on_open(client* c, websocketpp::connection_hdl hdl);

    /**
     * @brief Handles the WebSocket connection fail event.
     *
     * @param c Pointer to the WebSocket client.
     * @param hdl The connection handle.
     */
    void on_fail(client* c, websocketpp::connection_hdl hdl);

    /**
     * @brief Handles the WebSocket connection close event.
     *
     * @param c Pointer to the WebSocket client.
     * @param hdl The connection handle.
     */
    void on_close(client* c, websocketpp::connection_hdl hdl);

    /**
     * @brief Handles incoming WebSocket messages.
     *
     * @param c Pointer to the WebSocket client.
     * @param hdl The connection handle.
     * @param msg The received message.
     */
    void on_message(client* c, websocketpp::connection_hdl hdl, client::message_ptr msg);

    /**
     * @brief Handles subscription messages for a specific channel.
     *
     * @param channel The name of the channel.
     * @param data The JSON data received for the channel.
     */
    void handleSubscriptionMessage(const std::string& channel, const nlohmann::json& data);

    /**
     * @brief Handles authentication success messages.
     *
     * @param result The JSON result containing authentication details.
     */
    void on_message_auth(nlohmann::json result);

    /**
     * @brief Handles account summary messages.
     *
     * @param result The JSON result containing account summary details.
     */
    void on_message_summary(nlohmann::json result);

    /**
     * @brief Handles buy order success messages.
     *
     * @param order The JSON result containing buy order details.
     */
    void on_message_buy(nlohmann::json order);

    /**
     * @brief Handles order cancellation success messages.
     *
     * @param result The JSON result containing cancellation details.
     */
    void on_message_cancel(nlohmann::json result);

    /**
     * @brief Handles order book update messages.
     *
     * @param result The JSON result containing order book details.
     */
    void on_message_orderBook(nlohmann::json result);

    /**
     * @brief Handles order modification success messages.
     *
     * @param result The JSON result containing modification details.
     */
    void on_message_modify(nlohmann::json result);

    /**
     * @brief Handles position update messages.
     *
     * @param result The JSON result containing position details.
     */
    void on_message_positions(nlohmann::json result);

   

    client m_endpoint; ///< The WebSocket endpoint.
    websocketpp::connection_hdl m_hdl; ///< The connection handle.
    std::thread m_eventLoopThread; ///< The thread running the WebSocket event loop.
    bool m_connected; ///< Indicates whether the client is connected to the server.
    std::function<void()> m_authRequestCallback; ///< Callback function for authentication requests.
    bool m_authenticated; ///< Indicates whether the client is authenticated.
    bool m_waitingForResponse; ///< Indicates whether the client is waiting for a response.
    std::string m_accessToken; ///< The access token for authenticated sessions.
    std::map<std::string, std::string> m_lastData; ///< Stores the last received data for each channel.
    std::set<std::string> m_subscribedChannels; ///< Stores the names of subscribed channels.
};

#endif // WEBSOCKETCLIENT_H