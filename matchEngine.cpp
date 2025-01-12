#include <iostream>
#include <vector>
#include <map>
#include <mutex>

/*
    Disclaimer: Using C++11 for mutex and range-based for loops
    https://en.cppreference.com
*/

class Order
{
public:
    // do not alter signature of this constructor
    Order(const std::string &ordId,
          const std::string &secId,
          const std::string &side,
          const unsigned int qty,
          const std::string &user,
          const std::string &company)
        : m_orderId(ordId), m_securityId(secId), m_side(side), m_qty(qty), m_user(user), m_company(company) {}
    Order() {}
    // do not alter these accessor methods
    std::string orderId() const { return m_orderId; }
    std::string securityId() const { return m_securityId; }
    std::string side() const { return m_side; }
    std::string user() const { return m_user; }
    std::string company() const { return m_company; }
    unsigned int qty() const { return m_qty; }

private:
    // use the below to hold the order data
    // do not remove the these member variables
    std::string m_orderId;    // unique order id
    std::string m_securityId; // security identifier
    std::string m_side;       // side of the order, eg Buy or Sell
    unsigned int m_qty;       // qty for this order
    std::string m_user;       // user name who owns this order
    std::string m_company;    // company for user
};

class OrderCacheInterface
{
public:
    // implememnt the 6 methods below, do not alter signatures
    // add order to the cache
    virtual void addOrder(const Order &order) = 0;
    // remove order with this unique order id from the cache
    virtual void cancelOrder(const std::string &orderId) = 0;
    // remove all orders in the cache for this user
    virtual void cancelOrdersForUser(const std::string &user) = 0;
    // remove all orders in the cache for this security with qty >= minQty
    virtual void cancelOrdersForSecIdWithMinimumQty(const std::string &securityId, unsigned int minQty) = 0;
    // return the total qty that can match for the security id
    virtual unsigned int getMatchingSizeForSecurity(const std::string &securityId) = 0;
    // return all orders in cache in a vector
    virtual std::vector<Order> getAllOrders() const = 0;
    // virtual destructor, ensure proper cleanup and deallocations
    virtual ~OrderCacheInterface() {}
};

class OrderCache : public OrderCacheInterface
{
private:
    std::map<std::string, Order> orders;
    mutable std::mutex mutex; // Mutex for thread safety
    //using lock_guard: for the duration of a scoped block

public:
    void addOrder(const Order &order) override
    {
        std::lock_guard<std::mutex> lock(mutex);
        orders[order.orderId()] = order;
    }

    void cancelOrder(const std::string &orderId) override
    {
        std::lock_guard<std::mutex> lock(mutex);
        orders.erase(orderId);
    }

    void cancelOrdersForUser(const std::string &user) override
    {
        std::lock_guard<std::mutex> lock(mutex);
        for (auto it = orders.begin(); it != orders.end();)
        {
            if (it->second.user() == user)
                it = orders.erase(it);
            else
                ++it;
        }
    }

    //Do not keep in the book the resting quantity, if minQty is smaller than the original order
    void cancelOrdersForSecIdWithMinimumQty(const std::string &securityId, unsigned int minQty) override
    {
        std::lock_guard<std::mutex> lock(mutex);
        for (auto it = orders.begin(); it != orders.end();)
        {
            if (it->second.securityId() == securityId && it->second.qty() >= minQty)
                it = orders.erase(it);
            else
                ++it;
        }
    }

    unsigned int getMatchingSizeForSecurity(const std::string& securityId) override
    {
        std::lock_guard<std::mutex> lock(mutex);
        std::map<std::string, std::map<std::string, unsigned int>> orderBook;
        std::map<std::string, std::string> userCompanyMap; 

        // Build the order book
        for (const auto& order : orders)
        {
            if (order.second.securityId() == securityId)
            {
                if (orderBook.find(order.second.side()) == orderBook.end())
                    orderBook[order.second.side()] = std::map<std::string, unsigned int>();

                //updates the corresponding quantity stored in the orderBook. 
                //It adds the quantity of the current order (order.second.qty()) to the existing quantity 
                //for the specified user in the order book.
                orderBook[order.second.side()][order.second.user()] += order.second.qty();

                // Store user-company map
                userCompanyMap[order.second.user()] = order.second.company();
            }
        }

        unsigned int totalQty = 0;

        // Match orders
        if (orderBook.find("Buy") != orderBook.end())
        {
            //get buy Orders
            auto& buyOrders = orderBook["Buy"];

            // Temporary vector to hold sell orders to be removed
            std::vector<std::string> buyOrdersToRemove;

            for (auto buyIt = buyOrders.begin(); buyIt != buyOrders.end(); ++buyIt)
            {
                const auto& buyOrderUser = buyIt->first;
                auto& buyOrderQty = buyIt->second;

                //get sell Orders
                auto& sellOrders = orderBook["Sell"];

                // Temporary vector to hold sell orders to be removed
                std::vector<std::string> sellOrdersToRemove;

                for (auto sellIt = sellOrders.begin(); sellIt != sellOrders.end(); ++sellIt)
                {
                    const auto& sellOrderUser = sellIt->first;
                    auto& sellOrderQty = sellIt->second;

                    //check if orders are from the same company
                    if (userCompanyMap[buyOrderUser] != userCompanyMap[sellOrderUser])
                    {
                        unsigned int matchedQty = std::min(buyOrderQty, sellOrderQty);
                        totalQty += matchedQty;

                        // Reduce the matched quantity from buy and sell orders
                        buyOrderQty -= matchedQty;
                        sellOrderQty -= matchedQty;

                        // Mark sell order for removal if fully matched
                        if (sellOrderQty == 0)
                            sellOrdersToRemove.push_back(sellOrderUser);
                    }
                }

                // Remove marked sell orders after the inner loop
                for (const auto& orderToRemove : sellOrdersToRemove)
                    sellOrders.erase(orderToRemove);

                // Mark buy order for removal if fully matched
                if (buyOrderQty == 0)
                    buyOrdersToRemove.push_back(buyOrderUser);
            }

            // Remove marked buy orders after the outer loop
            for (const auto& orderToRemove : buyOrdersToRemove)
                buyOrders.erase(orderToRemove);
        }

        return totalQty;
    }

    std::vector<Order> getAllOrders() const override
    {
        std::lock_guard<std::mutex> lock(mutex);
        std::vector<Order> allOrders;
        allOrders.reserve(orders.size());

        for (const auto &order : orders)
        {
            allOrders.push_back(order.second);
        }

        return allOrders;
    }
};

