/**
 * @file main.cpp
 * @brief Main application for interacting with a WebSocket server.
 *
 * This file contains the main application logic for connecting to a WebSocket server,
 * sending requests, and handling responses. It provides a menu-driven interface for
 * performing various actions such as placing orders, subscribing to channels, and
 * retrieving account information.
 */

#include "webSocketClient.h"
#include "deriapi.h"
#include <fmt/core.h> 
#include <iostream>
#include <thread>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

/**
 * @brief Displays the main menu options.
 */
void showMenu() {
    fmt::print("\nMenu:\n");
    fmt::print("1. Autherise\n");
    fmt::print("2. Get Account Summary\n");
    fmt::print("3. Place a Buy Order\n");
    fmt::print("4. Cancel Order\n");
    fmt::print("5. Get Order Book\n");
    fmt::print("6. Modify Order\n");
    fmt::print("7. View Current Positions\n");
    fmt::print("8. Subscribe to Channel\n");
    fmt::print("9. Unsubscribe from Channel\n");
    fmt::print("10. Exit\n");
    fmt::print("Enter your choice: ");
}

/**
 * @brief Main function for the WebSocket client application.
 *
 * @return int Returns 0 on successful execution.
 */
int main() {
    webSocketClient client;

    int choice;
    do {
        showMenu();
        std::cin >> choice;

        switch (choice) {
            case 1: {
                std::string clientId;
                std::string clientSecret;
                fmt::print("Enter Client Id: ");
                std::cin>>clientId;
                fmt::print("Enter a Client Secret: ");
                std::cin>>clientSecret;
                client.setAuthRequestCallback([&client, clientId, clientSecret]() {
                    std::string authRequest = deriapi::authorize(clientId, clientSecret);
                    client.send(authRequest);
                });
                 std::string uri = "wss://test.deribit.com/ws/api/v2";
                client.connect(uri);

                // Wait for authentication to complete
                while (!client.isAuthenticated()) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
                break;
            }
            case 2: {
                std::string currency;
                fmt::print("Enter Currency: ");
                std::cin >> currency;
                std::string accountSummaryRequest = deriapi::getAccountSummary(currency);
                client.send(accountSummaryRequest);
                while (client.isWaitingForResponse()) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
                break;
            }
            case 3: {
                std::string instrument;
                fmt::print("Enter instrument name: ");
                std::cin >> instrument;

                int amount;
                fmt::print("Enter amount: ");
                std::cin >> amount;

                std::string orderType;
                fmt::print("Enter order type (limit, market, stop_limit, etc.): ");
                std::cin >> orderType;

                int price = 0;
                if (orderType == "limit" || orderType == "stop_limit") {
                    fmt::print("Enter price: ");
                    std::cin >> price;
                }

                std::string timeInForce;
                fmt::print("Enter time-in-force (good_til_cancelled, fill_or_kill, etc.): ");
                std::cin >> timeInForce;

                std::string label;
                fmt::print("Enter label: ");
                std::cin >> label;

                std::string buyRequest = deriapi::buyOrder(instrument, amount, orderType, price, timeInForce, label, client.getAccessToken());
                client.send(buyRequest);
                while (client.isWaitingForResponse()) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
                break;
            }
           
            case 4: {
                std::string orderId;
                fmt::print("Enter order id: ");
                std::cin >> orderId;
                std::string cancelRequest = deriapi::cancelOrder(orderId);
                client.send(cancelRequest);
                while (client.isWaitingForResponse()) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
                break;
            }
            case 5: {
                std::string instrumentName;
                int depth;
                fmt::print("Enter Instrument Name (e.g., BTC-PERPETUAL): ");
                std::cin >> instrumentName;
                fmt::print("Enter depth: (if want to skip, enter 0; default is 20): ");
                std::cin >> depth;
                if (depth == 0) depth = 20;
                std::string orderBookRequest = deriapi::getOrderBook(instrumentName, depth);
                client.send(orderBookRequest);
                while (client.isWaitingForResponse()) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
                break;
            }
            case 6: {
                std::string orderId;
                fmt::print("Enter Order ID: ");
                std::cin >> orderId;

                int amount;
                fmt::print("Enter New Amount: ");
                std::cin >> amount;

                double price;
                fmt::print("Enter New Price: ");
                std::cin >> price;

                std::string timeInForce;
                fmt::print("Enter Time-in-Force (e.g., good_til_cancelled): ");
                std::cin >> timeInForce;

                std::string modifyRequest = deriapi::modifyOrder(orderId, amount, price, timeInForce);
                client.send(modifyRequest);
                while (client.isWaitingForResponse()) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
                break;
            }
            case 7: {
                std::string currency;
                fmt::print("Enter Currency (e.g., BTC): ");
                std::cin >> currency;

                std::string kind;
                fmt::print("Enter Instrument Type (e.g., future, option, spot): ");
                std::cin >> kind;

                std::string positionsRequest = deriapi::getPositions(currency, kind);
                client.send(positionsRequest);
                while (client.isWaitingForResponse()) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
                break;
            }
            case 8: {
                std::string channel;
                fmt::print("Enter channel (e.g., ticker.BTC-PERPETUAL.100ms): ");
                std::cin >> channel;
                client.subscribe(channel);
                break;
            }
            case 9: {
                std::string channel;
                fmt::print("Enter channel to unsubscribe: ");
                std::cin >> channel;
                client.unsubscribe(channel);
                break;
            }
            case 10:
                fmt::print("Exiting...\n");
                break;
            default:
                fmt::print("Invalid choice. Please try again.\n");
                break;
        }
    } while (choice > 0 && choice < 10);

    // Close the WebSocket connection
    client.close();
    return 0;
}