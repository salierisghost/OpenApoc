#include "framework/configfile.h"
#include "framework/logger.h"
#include "library/xorshift.h"

#define BOOST_TEST_ALTERNATIVE_INIT_API
#include <boost/test/included/unit_test.hpp>

using namespace OpenApoc;

BOOST_AUTO_TEST_CASE(testExpectedValues)
{
	Xorshift128Plus<uint64_t> rng{};

	uint64_t r1 = rng();
	uint64_t r2 = rng();

	uint64_t expected_r1 = 0x03aacfee1f751183;
	uint64_t expected_r2 = 0xcb8aa3521c8fc259;
	uint64_t expected_r3 = 0xdd420b258a17fa82;

	BOOST_TEST(r1 == expected_r1,
	           format("unexpected r1 0x%016x, expected 0x%016x", r1, expected_r1));
	BOOST_TEST(r2 == expected_r2,
	           format("unexpected r2 0x%016x, expected 0x%016x", r2, expected_r2));

	// Save the state to another rng and check that result matches

	uint64_t s[2];
	Xorshift128Plus<uint64_t> rng2{};

	rng.getState(s);
	rng2.setState(s);

	uint64_t r3 = rng2();
	BOOST_TEST(r3 == expected_r3,
	           format("unexpected r3 0x%016x, expected 0x%016x", r3, expected_r3));
}

BOOST_AUTO_TEST_CASE(testDistribution)
{
	Xorshift128Plus<uint64_t> rng{};

	constexpr int num_test_buckets = 4;
	constexpr int num_test_iterations = 500000;

	unsigned buckets[num_test_buckets];
	for (int i = 0; i < num_test_buckets; i++)
	{
		buckets[i] = 0;
	}

	for (int i = 0; i < num_test_iterations; i++)
	{
		auto value = randBoundsExclusive(rng, 0, num_test_buckets);
		buckets[value]++;
	}

	LogWarning("RNG buckets:");
	for (int i = 0; i < num_test_buckets; i++)
	{
		LogWarning("%d:\t%u", i, buckets[i]);
	}
}

bool init_unit_test()
{
	auto &suite = boost::unit_test::framework::master_test_suite();

	return !config().parseOptions(suite.argc, suite.argv);
}
