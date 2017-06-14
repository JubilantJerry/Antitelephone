#ifndef QUERY_RESULT_H
#define QUERY_RESULT_H

#include <string>
#include <utility>

/**
 * @brief A class representing a server's response to a query.
 */
class QueryResult {
  public:
    /**
     * @brief Constructor.
     * @param success           Whether the query was a success.
     * @param response_tag      String tag associated with the response.
     */
    QueryResult(bool success, std::string const& response_tag = "")
        :success_{success},
         response_tag_{response_tag} {}

    /**
     * @brief Implicit cast to the underlying success value.
     */
    operator bool() const noexcept {
        return success_;
    }

    /**
     * @brief Accessor for the success value.
     * @return Whether the query was a success.
     */
    bool success() const noexcept {
        return success_;
    }

    /**
     * @brief Accessor for the response tag.
     * @return Tag associated with the response.
     */
    std::string const& response_tag() const noexcept {
        return response_tag_;
    }

  private:
    bool success_;
    std::string response_tag_;
};

#endif //QUERY_RESULT_H
