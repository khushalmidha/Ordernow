/**
 * @file deriapi.h
 * @brief Header file for the Deribit API wrapper.
 *
 * This file defines the functions to interact with the Deribit API, including
 * authorization, account management, order placement, and WebSocket subscriptions.
 */

#ifndef DERIAPI_H
#define DERIAPI_H

#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace deriapi {

    /**
     * @brief Authorizes the client using client ID and secret.
     *
     * This function generates a timestamp, nonce, and client signature, then creates
     * an authorization request JSON object using the "client_signature" grant type.
     *
     * @param clientId The client ID for authentication.
     * @param clientSecret The client secret for authentication.
     * @return std::string The authorization request in JSON format.
     * @throws std::runtime_error If the client signature cannot be generated.
     */
    std::string authorize(const std::string& clientId, const std::string& clientSecret);

    /**
     * @brief Retrieves the account summary for a specific currency.
     *
     * This function creates a JSON request to get the account summary for the specified currency.
     *
     * @param currency The currency for which to retrieve the account summary (e.g., "BTC").
     * @return std::string The account summary request in JSON format.
     */
    std::string getAccountSummary(const std::string& currency);

    /**
     * @brief Creates a buy order request.
     *
     * This function generates a JSON request for placing a buy order with the specified parameters.
     *
     * @param instrument The instrument name (e.g., "BTC-PERPETUAL").
     * @param amount The amount of the instrument to buy.
     * @param orderType The type of order (e.g., "limit", "market", "stop_limit").
     * @param price The price for limit or stop-limit orders.
     * @param timeInForce The time-in-force for the order (e.g., "good_til_cancelled").
     * @param label A custom label for the order.
     * @param accessToken The access token for authentication.
     * @return std::string The buy order request in JSON format.
     */
    std::string buyOrder(const std::string& instrument, int amount, const std::string& orderType, int price, const std::string& timeInForce, const std::string& label, const std::string& accessToken);

    /**
     * @brief Creates a cancel order request.
     *
     * This function generates a JSON request for canceling an order using the specified order ID.
     *
     * @param orderId The ID of the order to cancel.
     * @return std::string The cancel order request in JSON format.
     */
    std::string cancelOrder(const std::string& orderId);

    /**
     * @brief Creates a request to retrieve the order book for a specific instrument.
     *
     * This function generates a JSON request to get the order book for the specified instrument and depth.
     *
     * @param instrumentName The name of the instrument (e.g., "BTC-PERPETUAL").
     * @param depth The depth of the order book to retrieve.
     * @return std::string The order book request in JSON format.
     */
    std::string getOrderBook(const std::string& instrumentName, int depth);

    /**
     * @brief Creates a request to modify an existing order.
     *
     * This function generates a JSON request to modify an order with the specified parameters.
     *
     * @param orderId The ID of the order to modify.
     * @param amount The new amount for the order.
     * @param price The new price for the order.
     * @param timeInForce The new time-in-force for the order.
     * @param postOnly [optional] Whether the order should be post-only (default: false).
     * @param reduceOnly [optional] Whether the order should be reduce-only (default: false).
     * @return std::string The modify order request in JSON format.
     */
    std::string modifyOrder(const std::string& orderId, int amount, double price, const std::string& timeInForce, bool postOnly = false, bool reduceOnly = false);

    /**
     * @brief Creates a request to retrieve positions for a specific currency and kind.
     *
     * This function generates a JSON request to get positions for the specified currency and kind.
     *
     * @param currency The currency for which to retrieve positions (e.g., "BTC").
     * @param kind [optional] The kind of positions to retrieve (e.g., "future", "option"). Default: "future".
     * @return std::string The positions request in JSON format.
     */
    std::string getPositions(const std::string& currency, const std::string& kind = "future");

  

    /**
     * @brief Creates a request to subscribe to a WebSocket channel.
     *
     * This function generates a JSON request to subscribe to the specified WebSocket channel.
     *
     * @param channel The channel to subscribe to (e.g., "ticker.BTC-PERPETUAL.100ms").
     * @return std::string The subscription request in JSON format.
     */
    std::string subscribeToChannel(const std::string& channel);

    /**
     * @brief Creates a request to unsubscribe from a WebSocket channel.
     *
     * This function generates a JSON request to unsubscribe from the specified WebSocket channel.
     *
     * @param channel The channel to unsubscribe from (e.g., "ticker.BTC-PERPETUAL.100ms").
     * @return std::string The unsubscription request in JSON format.
     */
    std::string unsubscribeFromChannel(const std::string& channel);
}

#endif // DERIAPI_H