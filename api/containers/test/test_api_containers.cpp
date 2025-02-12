#include "container_controller.h"
#include "system.h"
#include <chrono>
#include <thread>
#include <drogon/drogon.h>
#include <gtest/gtest.h>

using namespace drogon;
using namespace nanoenv::system;

class ContainerControllerTest : public testing::Test {
protected:
    static std::shared_ptr<HttpClient> client;

    static void SetUpTestSuite() {
        client = HttpClient::newHttpClient("http://0.0.0.0:18080");

        const auto controller = std::make_shared<nanoenv::api::containers::ContainerController>();

        std::thread([&controller] {
            app()
                .addListener("0.0.0.0", 18080)
                .registerController(controller)
                .run();
        }).detach();

        waitForServerReady();
    }

    static void TearDownTestSuite() {
        app().quit();
    }

    static std::pair<ReqResult, HttpResponsePtr> sendRequestSync(
        const HttpRequestPtr &req
    ) {
        std::promise<std::pair<ReqResult, HttpResponsePtr>> promise;
        auto future = promise.get_future();

        client->sendRequest(req, [&promise](ReqResult result, const HttpResponsePtr &response) {
            promise.set_value(std::make_pair(result, response));
        });

        return future.get();
    }

private:
    static void waitForServerReady() {
        using namespace std::chrono_literals;
        constexpr int maxRetries = 50;
        constexpr auto retryInterval = 100ms;

        for (int i = 0; i < maxRetries; ++i) {
            auto req = HttpRequest::newHttpRequest();
            req->setMethod(Get);
            req->setPath("/containers/health");

            auto [result, response] = sendRequestSync(req);
            if (result == ReqResult::Ok && response->getStatusCode() == k200OK) {
                return;
            }

            std::this_thread::sleep_for(retryInterval);
        }

        app().quit();

        throw std::runtime_error("Server failed to start within the expected time.");
    }
};

std::shared_ptr<HttpClient> ContainerControllerTest::client = nullptr;

TEST_F(
    ContainerControllerTest,
    CreateContainer_Success
) {
    const std::string containerName = System::generateUUID();

    const auto req = HttpRequest::newHttpJsonRequest(R"({
        "name": ")" + containerName + R"(",
        "image": "alpine",
        "cpu": 2,
        "memory": 1024,
        "network": "bridge"
    })");

    req->setMethod(Post);
    req->setPath("/containers");

    auto [result, response] = sendRequestSync(req);

    EXPECT_EQ(result, drogon::ReqResult::Ok);
    EXPECT_EQ(response->getStatusCode(), drogon::k200OK);
}

TEST_F(
    ContainerControllerTest,
    StartContainer_Success
) {
    const std::string containerName = System::generateUUID();

    const auto createReq = HttpRequest::newHttpJsonRequest(R"({
        "name": ")" + containerName + R"(",
        "image": "alpine",
        "cpu": 2,
        "memory": 1024,
        "network": "bridge"
    })");
    createReq->setMethod(Post);
    createReq->setPath("/containers");
    sendRequestSync(createReq);

    const auto startReq = HttpRequest::newHttpRequest();
    startReq->setMethod(Post);
    startReq->setPath("/containers/" + containerName + "/start");

    auto [result, response] = sendRequestSync(startReq);

    EXPECT_EQ(result, drogon::ReqResult::Ok);
    EXPECT_EQ(response->getStatusCode(), drogon::k200OK);
}

TEST_F(
    ContainerControllerTest,
    StopContainer_Success
) {
    const std::string containerName = System::generateUUID();

    const auto createReq = HttpRequest::newHttpJsonRequest(R"({
        "name": ")" + containerName + R"(",
        "image": "alpine",
        "cpu": 2,
        "memory": 1024,
        "network": "bridge"
    })");
    createReq->setMethod(Post);
    createReq->setPath("/containers");
    sendRequestSync(createReq);

    const auto stopReq = HttpRequest::newHttpRequest();
    stopReq->setMethod(Post);
    stopReq->setPath("/containers/" + containerName + "/stop");

    auto [result, response] = sendRequestSync(stopReq);

    EXPECT_EQ(result, drogon::ReqResult::Ok);
    EXPECT_EQ(response->getStatusCode(), drogon::k200OK);
}

TEST_F(
    ContainerControllerTest,
    DestroyContainer_Success
) {
    const std::string containerName = System::generateUUID();

    const auto createReq = HttpRequest::newHttpJsonRequest(R"({
        "name": ")" + containerName + R"(",
        "image": "alpine",
        "cpu": 2,
        "memory": 1024,
        "network": "bridge"
    })");
    createReq->setMethod(Post);
    createReq->setPath("/containers");
    sendRequestSync(createReq);

    const auto destroyReq = HttpRequest::newHttpRequest();
    destroyReq->setMethod(Delete);
    destroyReq->setPath("/containers/" + containerName);

    auto [result, response] = sendRequestSync(destroyReq);

    EXPECT_EQ(result, drogon::ReqResult::Ok);
    EXPECT_EQ(response->getStatusCode(), drogon::k200OK);
}

TEST_F(
    ContainerControllerTest,
    DestroyContainer_Failure_NonExistent
) {
    const auto destroyReq = HttpRequest::newHttpRequest();
    destroyReq->setMethod(Delete);
    destroyReq->setPath("/containers/nonexistent-container");

    auto [result, response] = sendRequestSync(destroyReq);

    EXPECT_EQ(result, drogon::ReqResult::Ok);
    EXPECT_EQ(response->getStatusCode(), drogon::k404NotFound);
}
