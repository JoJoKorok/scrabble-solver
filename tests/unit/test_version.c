#include "scrabble/version.h"

#include "test.h"

static int reports_configured_project_version(void) {
    TEST_ASSERT_STRING(SCRABBLE_TEST_PROJECT_VERSION,
                       scrabble_solver_version());
    return 0;
}

static const ScrabbleTestCase TESTS[] = {
    {"reports configured project version", reports_configured_project_version}
};

TEST_MAIN(TESTS)
