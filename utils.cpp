/**
 * @file utils.cpp
 * @brief Implementation of utility functions for the Deribit API wrapper.
 *
 * This file contains utility functions for generating timestamps, nonces, and client signatures
 * required for interacting with the Deribit API.
 */

#include "utils.h"
#include <fmt/core.h> // Use fmt for formatted output
#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <chrono>
#include <random>
#include <iomanip>
#include <sstream>

namespace utils {

    /**
     * @brief Generates a timestamp in milliseconds since the Unix epoch.
     *
     * This function retrieves the current system time and converts it to milliseconds.
     *
     * @return std::string The timestamp as a string.
     */
    std::string getTimeStamp() {
        using namespace std::chrono;
        long long timeStamp = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
        return std::to_string(timeStamp);
    }

    /**
     * @brief Generates a random nonce of 8 characters.
     *
     * This function creates a random string of 8 characters using alphanumeric characters.
     *
     * @return std::string The generated nonce.
     */
    std::string getNonce() {
        const std::string chars = "abcdefghijklmnopqrstuvwxyz0123456789";
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<char> dist(0, chars.size() - 1);

        std::string nonce;
        for (int i = 0; i < 8; ++i) {
            nonce += chars[dist(gen)];
        }
        return nonce;
    }

    /**
     * @brief Converts binary data to a hexadecimal string.
     *
     * This function takes binary data and converts it to a hexadecimal representation.
     *
     * @param data The binary data to convert.
     * @param length The length of the binary data.
     * @return std::string The hexadecimal representation of the data.
     */
    std::string toHex(const unsigned char* data, size_t length) {
        std::ostringstream hexStream;
        hexStream << std::hex << std::setfill('0');
        for (size_t i = 0; i < length; ++i) {
            hexStream << std::setw(2) << (int)data[i];
        }
        return hexStream.str();
    }

    /**
     * @brief Computes the HMAC-SHA256 hash of the given data using the provided secret.
     *
     * This function uses OpenSSL's HMAC function to compute the HMAC-SHA256 hash.
     *
     * @param secret The secret key for the HMAC computation.
     * @param data The data to hash.
     * @return std::string The HMAC-SHA256 hash as a hexadecimal string.
     */
    std::string hmacSha256(const std::string& secret, const std::string& data) {
        unsigned char result[EVP_MAX_MD_SIZE];
        unsigned int resultLength = 0;

        HMAC(EVP_sha256(), secret.c_str(), secret.length(),
             reinterpret_cast<const unsigned char*>(data.c_str()), data.length(),
             result, &resultLength);

        return toHex(result, resultLength);
    }

    /**
     * @brief Generates a client signature using the provided client secret, timestamp, nonce, and data.
     *
     * This function creates a string to sign by concatenating the timestamp, nonce, and data,
     * then computes the HMAC-SHA256 hash of the string using the client secret.
     *
     * @param clientSecret The client secret for the HMAC computation.
     * @param timeStamp The timestamp to include in the signature.
     * @param nonce The nonce to include in the signature.
     * @param data Additional data to include in the signature (optional).
     * @return std::string The client signature as a hexadecimal string.
     */
    std::string getClientSignature(const std::string& clientSecret, const std::string& timeStamp, const std::string& nonce, const std::string& data) {
        std::string stringToSign = timeStamp + "\n" + nonce + "\n" + data;
        return hmacSha256(clientSecret, stringToSign);
    }
}