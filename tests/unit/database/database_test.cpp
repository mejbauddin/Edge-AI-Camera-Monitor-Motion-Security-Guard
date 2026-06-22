#include <gtest/gtest.h>

#include "Database.hpp"

#include <filesystem>

using csx::database::createDatabase;

TEST(DatabaseTest, OpensAndMigratesSchema) {
    const auto path = (std::filesystem::temp_directory_path() / "csx_db_test.sqlite").string();
    std::filesystem::remove(path);

    auto db = createDatabase();
    ASSERT_TRUE(db->open(path));
    EXPECT_TRUE(db->isOpen());
    EXPECT_EQ(db->health().status, csx::core::EngineStatus::Online);

    db->close();
    std::filesystem::remove(path);
}

TEST(DatabaseTest, StoresUsersAndAuthorizedFaces) {
    const auto path = (std::filesystem::temp_directory_path() / "csx_db_faces_test.sqlite").string();
    std::filesystem::remove(path);

    auto db = createDatabase();
    ASSERT_TRUE(db->open(path));

    const auto userId = db->users().createUser("Operator Alpha", "operator");
    ASSERT_TRUE(userId.has_value());

    std::vector<float> embedding(512, 0.1F);
    embedding[0] = 0.9F;
    const auto faceId = db->faces().addAuthorizedFace(*userId, embedding);
    ASSERT_TRUE(faceId.has_value());

    const auto faces = db->faces().listAuthorizedFaces();
    ASSERT_EQ(faces.size(), 1U);
    EXPECT_EQ(faces[0].userName, "Operator Alpha");
    EXPECT_NEAR(faces[0].embedding[0], 0.9F, 0.001F);

    db->close();
    std::filesystem::remove(path);
}

TEST(DatabaseTest, PersistsEventsAndThreats) {
    const auto path = (std::filesystem::temp_directory_path() / "csx_db_events_test.sqlite").string();
    std::filesystem::remove(path);

    auto db = createDatabase();
    ASSERT_TRUE(db->open(path));

    ASSERT_TRUE(db->events().insertEvent("camera.state", "primary", R"({"state":"online"})"));
    ASSERT_TRUE(db->threats().insertThreat("YELLOW", 35.0F, "Motion detected", "[1]"));

    EXPECT_EQ(db->events().recentEvents().size(), 1U);
    EXPECT_EQ(db->threats().recentThreats().size(), 1U);

    db->close();
    std::filesystem::remove(path);
}
