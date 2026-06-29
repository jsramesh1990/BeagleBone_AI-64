#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <QSignalSpy>
#include <QTimer>
#include "gui-app/Dashboard.h"
#include "gui-app/DataManager.h"

class DashboardTest : public ::testing::Test {
protected:
    void SetUp() override {
        dashboard = new Dashboard();
        // Initialize with test data
        QTimer::singleShot(100, [this]() {
            dashboard->dataManager()->startUpdates();
        });
    }
    
    void TearDown() override {
        dashboard->dataManager()->stopUpdates();
        delete dashboard;
    }
    
    Dashboard* dashboard;
};

TEST_F(DashboardTest, Constructor) {
    EXPECT_NE(dashboard, nullptr);
    EXPECT_NE(dashboard->dataManager(), nullptr);
    EXPECT_NE(dashboard->deviceController(), nullptr);
    EXPECT_NE(dashboard->settingsManager(), nullptr);
    EXPECT_NE(dashboard->themeManager(), nullptr);
}

TEST_F(DashboardTest, TitleProperty) {
    QString testTitle = "Test Dashboard";
    dashboard->setTitle(testTitle);
    EXPECT_EQ(dashboard->title(), testTitle);
}

TEST_F(DashboardTest, FullScreenProperty) {
    EXPECT_FALSE(dashboard->fullScreen());
    dashboard->setFullScreen(true);
    EXPECT_TRUE(dashboard->fullScreen());
}

TEST_F(DashboardTest, CurrentViewProperty) {
    QString testView = "sensors";
    dashboard->setCurrentView(testView);
    EXPECT_EQ(dashboard->currentView(), testView);
}

TEST_F(DashboardTest, RefreshIntervalProperty) {
    int testInterval = 2000;
    dashboard->setRefreshInterval(testInterval);
    EXPECT_EQ(dashboard->refreshInterval(), testInterval);
}

TEST_F(DashboardTest, ShowView) {
    QSignalSpy spy(dashboard, &Dashboard::viewChanged);
    dashboard->showView("charts");
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.takeFirst().at(0).toString(), "charts");
}

TEST_F(DashboardTest, ToggleFullScreen) {
    QSignalSpy spy(dashboard, &Dashboard::fullScreenChanged);
    dashboard->toggleFullScreen();
    EXPECT_EQ(spy.count(), 1);
    EXPECT_TRUE(dashboard->fullScreen());
    dashboard->toggleFullScreen();
    EXPECT_EQ(spy.count(), 2);
    EXPECT_FALSE(dashboard->fullScreen());
}

TEST_F(DashboardTest, ToggleTheme) {
    QSignalSpy spy(dashboard, &Dashboard::themeChanged);
    dashboard->toggleTheme();
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(DashboardTest, DataManagerConnection) {
    QSignalSpy spy(dashboard->dataManager(), &DataManager::dataUpdated);
    dashboard->dataManager()->updateData();
    // Wait for async processing
    QTest::qWait(100);
    EXPECT_GT(spy.count(), 0);
}

TEST_F(DashboardTest, DataManagerStartStop) {
    dashboard->dataManager()->startUpdates();
    EXPECT_TRUE(dashboard->dataManager()->m_updateTimer.isActive());
    dashboard->dataManager()->stopUpdates();
    EXPECT_FALSE(dashboard->dataManager()->m_updateTimer.isActive());
}
