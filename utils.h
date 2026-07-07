/**
 * @file utils.h
 * @brief Header file for utility functions used in the Deribit API wrapper.
 *
 * This file defines utility functions for generating timestamps, nonces, and client signatures
 * required for interacting with the Deribit API.
 */

#ifndef UTILS_H
#define UTILS_H

#include <string>

namespace utils {

    /**
     * @brief Generates a timestamp in milliseconds since the Unix epoch.
     *
     * This function retrieves the current system time and converts it to milliseconds.
     *
     * @return std::string The timestamp as a string.
     */
    std::string getTimeStamp();

    /**
     * @brief Generates a random nonce of 8 characters.
     *
     * This function creates a random string of 8 characters using alphanumeric characters.
     *
     * @return std::string The generated nonce.
     */
    std::string getNonce();

    /**
     * @brief Generates a client signature using the provided client secret, timestamp, nonce, and data.
     *
     * This function creates a string to sign by concatenating the timestamp, nonce, and data,
     * then computes the HMAC-SHA256 hash of the string using the client secret.
     *
     * @param clientSecret The client secret for the HMAC computation.
     * @param timeStamp The timestamp to include in the signature.
     * @param nonce The nonce to include in the signature.
     * @param data [optional] Additional data to include in the signature.
     * @return std::string The client signature as a hexadecimal string.
     */
    std::string getClientSignature(const std::string& clientSecret, const std::string& timeStamp, const std::string& nonce, const std::string& data = "");

    /**
     * @brief Converts binary data to a hexadecimal string.
     *
     * This function takes binary data and converts it to a hexadecimal representation.
     *
     * @param data The binary data to convert.
     * @param length The length of the binary data.
     * @return std::string The hexadecimal representation of the data.
     */
    std::string toHex(const unsigned char* data, size_t length);

    /**
     * @brief Computes the HMAC-SHA256 hash of the given data using the provided secret.
     *
     * This function uses OpenSSL's HMAC function to compute the HMAC-SHA256 hash.
     *
     * @param secret The secret key for the HMAC computation.
     * @param data The data to hash.
     * @return std::string The HMAC-SHA256 hash as a hexadecimal string.
     */
    std::string hmacSha256(const std::string& secret, const std::string& data);
}

#endif // UTILS_H