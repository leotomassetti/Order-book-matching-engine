#include <chrono>
#include "matchEngine.cpp"
using namespace std;

void runPerformanceTest()
{
    const int numOrders = 1000000;  // Number of orders to add
    const std::string securityId = "SecId";
    const std::string side = "Buy";
    const unsigned int qty = 1000;
    const std::string user = "User";
    const std::string company = "Company";

    // Create the order cache
    OrderCache orderCache;

    // Start the timer
    auto startTime = std::chrono::high_resolution_clock::now();

    // Add a large number of orders to the cache
    for (int i = 0; i < numOrders; ++i)
    {
        std::string orderId = "OrdId" + std::to_string(i);
        std::string secId = securityId + std::to_string(i);
        std::string usr = user + std::to_string(i);

        Order order(orderId, secId, side, qty, usr, company);
        orderCache.addOrder(order);
    }

    // Stop the timer
    auto endTime = std::chrono::high_resolution_clock::now();

    // Calculate the elapsed time
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

    // Print the result
    std::cout << "Added " << numOrders << " orders to the cache in " << duration << " milliseconds." << std::endl;
}

int main()
{
    runPerformanceTest();

    return 0;
}
