#include "framework/configfile.h"
#include "framework/data.h"
#include "framework/framework.h"
#include "framework/image.h"
#include "framework/logger.h"
#include <map>

#define BOOST_TEST_ALTERNATIVE_INIT_API
#include <boost/test/included/unit_test.hpp>

#include <boost/test/data/test_case.hpp>

using namespace OpenApoc;

static bool testImage(const UString &imageName, const UString &referenceName)
{
	auto img = fw().data->loadImage(imageName);
	auto reference = fw().data->loadImage(referenceName);
	if (!img)
	{
		LogWarning("Failed to load image");
		return false;
	}
	if (!reference)
	{
		LogWarning("Failed to load reference");
		return false;
	}

	auto rgbImg = std::dynamic_pointer_cast<RGBImage>(img);
	if (!rgbImg)
	{
		LogWarning("Image not RGBImage");
		return false;
	}

	auto rgbReference = std::dynamic_pointer_cast<RGBImage>(reference);
	if (!rgbReference)
	{
		LogWarning("Reference not RGB image");
		return false;
	}

	if (img->size != reference->size)
	{
		LogWarning("Invalid size, %s doesn't match reference %s", img->size, reference->size);
		return false;
	}

	RGBImageLock imgLock(rgbImg, ImageLockUse::Read);
	RGBImageLock refLock(rgbReference, ImageLockUse::Read);

	for (unsigned int y = 0; y < img->size.y; y++)
	{
		for (unsigned int x = 0; x < img->size.x; x++)
		{
			auto i = imgLock.get({x, y});
			auto r = refLock.get({x, y});
			if (i != r)
			{
				LogWarning(
				    "Image mismatch at {%d,%d} (RGBA img {%d,%d,%d,%d} != RGBA ref {%d,%d,%d,%d}",
				    x, y, (int)i.r, (int)i.g, (int)i.b, (int)i.a, (int)r.r, (int)r.g, (int)r.b,
				    (int)r.a);

				return false;
			}
		}
	}
	return true;
}

typedef std::pair<const UString, UString> pair_map_t;
BOOST_TEST_DONT_PRINT_LOG_VALUE(pair_map_t)

const std::map<UString, UString> pcxImages = {
    {"xcom3/ufodata/titles.pcx", "test_images/ufodata_titles.png"}};
BOOST_DATA_TEST_CASE(testPcx, pcxImages, imagePair)
{
	BOOST_TEST(
	    testImage(imagePair.first, imagePair.second),
	    format("Image \"%s\" didn't match reference \"%s\"", imagePair.first, imagePair.second));
}

const std::map<UString, UString> rawImages = {
    {"RAW:xcom3/ufodata/isobord1.dat:640:128:xcom3/ufodata/pal_01.dat",
     "test_images/ufodata_isobord1_pal01.png"}};
BOOST_DATA_TEST_CASE(testRaw, rawImages, imagePair)
{
	BOOST_TEST(
	    testImage(imagePair.first, imagePair.second),
	    format("Image \"%s\" didn't match reference \"%s\"", imagePair.first, imagePair.second));
}

const std::map<UString, UString> pckImages = {
    {"PCK:xcom3/ufodata/newbut.pck:xcom3/ufodata/newbut.tab:30:xcom3/ufodata/base.pcx",
     "test_images/ufodata_newbut_3_base_pcx.png"},
    {"PCK:xcom3/ufodata/city.pck:xcom3/ufodata/city.tab:956:xcom3/ufodata/pal_01.dat",
     "test_images/ufodata_city_956_pal01.png"},
    {"PCK:xcom3/tacdata/unit/xcom1a.pck:xcom3/tacdata/unit/xcom1a.tab:30:xcom3/tacdata/"
     "tactical.pal",
     "test_images/tacdata_unit_xcom1a_30_tactical.png"},
    {"PCK:xcom3/tacdata/unit/xcom1a.pck:xcom3/tacdata/unit/xcom1a.tab:240:xcom3/tacdata/"
     "tactical.pal",
     "test_images/tacdata_unit_xcom1a_240_tactical.png"},
    {"PCK:xcom3/tacdata/unit/xcom1a.pck:xcom3/tacdata/unit/xcom1a.tab:134:xcom3/tacdata/"
     "tactical.pal",
     "test_images/tacdata_unit_xcom1a_134_tactical.png"}};
BOOST_DATA_TEST_CASE(testPck, pckImages, imagePair)
{
	BOOST_TEST(
	    testImage(imagePair.first, imagePair.second),
	    format("Image \"%s\" didn't match reference \"%s\"", imagePair.first, imagePair.second));
}

const std::map<UString, UString> stratImages = {
    {"PCKSTRAT:xcom3/ufodata/stratmap.pck:xcom3/ufodata/stratmap.tab:32:xcom3/ufodata/"
     "pal_01.dat",
     "test_images/ufodata_stratmap_32.png"}};
BOOST_DATA_TEST_CASE(testStrat, stratImages, imagePair)
{
	BOOST_TEST(
	    testImage(imagePair.first, imagePair.second),
	    format("Image \"%s\" didn't match reference \"%s\"", imagePair.first, imagePair.second));
}

const std::map<UString, UString> shadowImages = {
    {"PCKSHADOW:xcom3/ufodata/shadow.pck:xcom3/ufodata/shadow.tab:5:xcom3/ufodata/pal_01.dat",
     "test_images/ufodata_shadow_5.png"}};
BOOST_DATA_TEST_CASE(testShadow, shadowImages, imagePair)
{
	BOOST_TEST(
	    testImage(imagePair.first, imagePair.second),
	    format("Image \"%s\" didn't match reference \"%s\"", imagePair.first, imagePair.second));
}

const std::map<UString, UString> loftempsImages = {
    {"LOFTEMPS:xcom3/ufodata/loftemps.dat:xcom3/ufodata/loftemps.tab:113",
     "test_images/ufodata_loftemps_113.png"},
    {"LOFTEMPS:xcom3/ufodata/loftemps.dat:xcom3/ufodata/loftemps.tab:150",
     "test_images/ufodata_loftemps_150.png"},
    {"LOFTEMPS:xcom3/ufodata/loftemps.dat:xcom3/ufodata/loftemps.tab:151",
     "test_images/ufodata_loftemps_151.png"}};
BOOST_DATA_TEST_CASE(testLoftemps, loftempsImages, imagePair)
{
	BOOST_TEST(
	    testImage(imagePair.first, imagePair.second),
	    format("Image \"%s\" didn't match reference \"%s\"", imagePair.first, imagePair.second));
}

sp<Framework> f = nullptr;
bool init_unit_test()
{
	auto &suite = boost::unit_test::framework::master_test_suite();

	if (config().parseOptions(suite.argc, suite.argv))
	{
		return false;
	}
	f = mksp<Framework>("OpenApoc", false);
	return true;
}
