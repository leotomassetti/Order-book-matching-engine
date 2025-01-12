#include <chrono>
#include <random>
#include <sstream>

// Function to generate a random string
std::string generateRandomString(int length)
{
    static const std::string chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> distribution(0, chars.size() - 1);

    std::string result;
    result.reserve(length);

    for (int i = 0; i < length; ++i)
        result.push_back(chars[distribution(gen)]);

    return result;
}

// Function to generate a random order
Order generateRandomOrder(int index)
{
    std::string orderId = "OrdId" + std::to_string(index);
    std::string secId = "SecId" + std::to_string(index);
    std::string user = "User" + std::to_string(index);
    std::string company = "Company" + std::to_string(index);
    unsigned int qty = static_cast<unsigned int>(100 + rand() % 900);  // Random quantity between 100 and 999

    return Order(orderId, secId, "Buy", qty, user, company);
}

void runPerformanceTest()
{
    const int numOrders = 1000000;  // Number of orders to add

    // Create the order cache
    OrderCache orderCache;

    // Start the timer
    auto startTime = std::chrono::high_resolution_clock::now();

    // Add a large number of random orders to the cache
    for (int i = 0; i < numOrders; ++i)
    {
        Order order = generateRandomOrder(i);
        orderCache.addOrder(order);
    }

    // Stop the timer
    auto endTime = std::chrono::high_resolution_clock::now();

    // Calculate the elapsed time
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

    // Print the result
    std::cout << "Added " << numOrders << " random orders to the cache in " << duration << " milliseconds." << std::endl;
}

int main()
{
    runPerformanceTest();

    return 0;
}
