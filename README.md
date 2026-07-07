# High-Performance Order Execution and Management System

## Overview
This project is a **High-Performance Order Execution and Management System** designed to facilitate real-time trading operations with efficient order handling. It supports various trading functionalities such as order placement, modification, cancellation, and real-time market data subscription using WebSockets.

## Features
- **Real-time Order Execution**: Place, modify, and cancel orders instantly.
- **WebSocket Support**: Enables real-time market data streaming.
- **Authentication System**: Secure API-based authentication mechanism.
- **Account Management**: Retrieve account summary and position details.
- **High Performance**: Optimized for low-latency order execution.

## Installation
1. Clone the repository:
   ```bash
   git clone https://github.com/your-username/order-execution-system.git
   cd order-execution-system
   ```
2. Install dependencies manually (Ubuntu):
   ```bash
   sudo apt update
   sudo apt install -y build-essential cmake libssl-dev libboost-all-dev
   ```
3. Build the project:
   ```bash
   ./build_and_run.sh
   ```

## Usage
1. **Start the system:**
   ```bash
   ./DeriConsole
   ```
2. **Available Commands:**
   - Authenticate the client
   - Get account summary
   - Place a buy order
   - Cancel an order
   - Get order book details
   - Modify an existing order
   - View open positions
   - Subscribe/unsubscribe to market data channels

## API Functions
- **authorize(clientId, clientSecret)**: Authenticate client using API credentials.
- **buyOrder(instrument, amount, orderType, price, timeInForce, label, accessToken)**: Place a buy order.
- **cancelOrder(orderId)**: Cancel an existing order.
- **getOrderBook(instrumentName, depth)**: Retrieve order book data.
- **modifyOrder(orderId, amount, price, timeInForce, postOnly, reduceOnly)**: Modify an open order.
- **getPositions(currency, kind)**: Retrieve current open positions.
- **subscribeToChannel(channel)**: Subscribe to a WebSocket channel.
- **unsubscribeFromChannel(channel)**: Unsubscribe from a WebSocket channel.

## License
This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

## Contributions
Contributions and improvements are welcome! Feel free to submit issues or pull requests.

## Disclaimer
This system is intended for research and educational purposes only. Use it responsibly and ensure compliance with financial regulations.

