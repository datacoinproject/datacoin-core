// Copyright (c) 2012-2015 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "key.h"

#include "base58.h"
#include "script/script.h"
#include "uint256.h"
#include "util.h"
#include "utilstrencodings.h"
#include "test/test_bitcoin.h"

#include <string>
#include <vector>

#include <boost/test/unit_test.hpp>

//DATACOIN ADDED //DATACOIN CHANGED об набора адресов работают. 
//Первый набор сделал из изначально не compressed ключей
//static const std::string strSecret1 = "6K1gCRe1HEKgLgJs1GGPKhdg1UFfhRsPVfKBv4bMwvg1jGMB61q";
//static const std::string strSecret2 = "6KEjBhru5NKsZZbY8p1BcB9788ezwbF3YDJUdFwkAVrzYxNYs7x";
//static const std::string strSecret1C = "QSVz1RkLEdoSdrVqiANuXYLsB5HjoSmQHW7HBCrfTRuCDA4r1JCV";
//static const std::string strSecret2C = "QTVbCFMErH8JAisSRUTopFkn2Unijx8zCmJ8w2xdWvPf1Z6Bn21T";
//static const std::string addr1 = "DS4vCWCVGiB8VUBXYweexi2YdYdFT5w499";
//static const std::string addr2 = "DEWDrTsA9dCxhyS1Xo5DVWiWoeww8cVBQq";
//static const std::string addr1C = "DK1HiS2f1aGosPMQGJEa1pDKJfDH29Y2Ng";
//static const std::string addr2C = "DEMAtHe1PdvxcaLY6kB1Nwtpkpm6xrY6mU";

//Второй набор сделал из изначально compressed ключей
static const std::string strSecret1 = "6Kuvgivks4QNdVK19ComSa7x9K9UwPXeUay4RVRwsrMcqMMn4qi";
static const std::string strSecret2 = "6JcYxzHZrMzNA2CZp5P7P11eBHDJctd5jhbrYMHzdT4R7vr7DmD";
static const std::string strSecret1C = "QWUbxL4Gbx2AU3xTdD7R9sBgPh6SK4SGQkvS76V7DSr1AR1EENEY";
static const std::string strSecret2C = "QQjv67r7zseCNpJ3pnLbA73q6NZJmQrFuUCqfNa1Mt25TLW6Z67z";
static const std::string addr1 = "DKNzJgvhfoSV49LvJdrGK3mWrRogjgVoWW";
static const std::string addr2 = "DCikyYEPP52qFbKkN99rLGaxocCQuxUrTw";
static const std::string addr1C = "DDqueAuaKascEfAkS5xzp6eX1GBuVPAoGu";
static const std::string addr2C = "DU5m2wedme2yVZewkG3yXAcgTfBQbp4kiU";

static const std::string strAddressBad = "1HV9Lc3sNHZxwj4Zk6fB38tEmBryq2cBiF";


BOOST_FIXTURE_TEST_SUITE(key_tests, BasicTestingSetup)

BOOST_AUTO_TEST_CASE(key_test1)
{
    CBitcoinSecret bsecret1, bsecret2, bsecret1C, bsecret2C, baddress1;
    BOOST_CHECK( bsecret1.SetString (strSecret1));
    BOOST_CHECK( bsecret2.SetString (strSecret2));
    BOOST_CHECK( bsecret1C.SetString(strSecret1C));
    BOOST_CHECK( bsecret2C.SetString(strSecret2C));
    BOOST_CHECK(!baddress1.SetString(strAddressBad));

    CKey key1  = bsecret1.GetKey();
    BOOST_CHECK(key1.IsCompressed() == false);
    CKey key2  = bsecret2.GetKey();
    BOOST_CHECK(key2.IsCompressed() == false);
    CKey key1C = bsecret1C.GetKey();
    BOOST_CHECK(key1C.IsCompressed() == true);
    CKey key2C = bsecret2C.GetKey();
    BOOST_CHECK(key2C.IsCompressed() == true);

    CPubKey pubkey1  = key1. GetPubKey();
    CPubKey pubkey2  = key2. GetPubKey();
    CPubKey pubkey1C = key1C.GetPubKey();
    CPubKey pubkey2C = key2C.GetPubKey();

    //Так можно получить compressed и не compressed пары secret
    //CKey tmp; CBitcoinSecret tmpSec;
    //tmp.Set(key1.begin(), key1.end(), false); tmpSec.SetKey(tmp);
    //std::cerr << "\n" << tmpSec.ToString() << "\n";
    //tmp.Set(key2.begin(), key2.end(), false); tmpSec.SetKey(tmp);
    //std::cerr << "\n" << tmpSec.ToString() << "\n";
    //tmp.Set(key1C.begin(), key1C.end(), false); tmpSec.SetKey(tmp);
    //std::cerr << "\n" << tmpSec.ToString() << "\n";
    //tmp.Set(key2C.begin(), key2C.end(), false); tmpSec.SetKey(tmp);
    //std::cerr << "\n" << tmpSec.ToString() << "\n";
    //tmp.Set(key1.begin(), key1.end(), true); tmpSec.SetKey(tmp);
    //std::cerr << "\n" << tmpSec.ToString() << "\n";
    //tmp.Set(key2.begin(), key2.end(), true); tmpSec.SetKey(tmp);
    //std::cerr << "\n" << tmpSec.ToString() << "\n";
    //tmp.Set(key1C.begin(), key1C.end(), true); tmpSec.SetKey(tmp);
    //std::cerr << "\n" << tmpSec.ToString() << "\n";
    //tmp.Set(key2C.begin(), key2C.end(), true); tmpSec.SetKey(tmp);
    //std::cerr << "\n" << tmpSec.ToString() << "\n";	

    BOOST_CHECK(key1.VerifyPubKey(pubkey1));
    BOOST_CHECK(!key1.VerifyPubKey(pubkey1C));
    BOOST_CHECK(!key1.VerifyPubKey(pubkey2));
    BOOST_CHECK(!key1.VerifyPubKey(pubkey2C));

    BOOST_CHECK(!key1C.VerifyPubKey(pubkey1));
    BOOST_CHECK(key1C.VerifyPubKey(pubkey1C));
    BOOST_CHECK(!key1C.VerifyPubKey(pubkey2));
    BOOST_CHECK(!key1C.VerifyPubKey(pubkey2C));

    BOOST_CHECK(!key2.VerifyPubKey(pubkey1));
    BOOST_CHECK(!key2.VerifyPubKey(pubkey1C));
    BOOST_CHECK(key2.VerifyPubKey(pubkey2));
    BOOST_CHECK(!key2.VerifyPubKey(pubkey2C));

    BOOST_CHECK(!key2C.VerifyPubKey(pubkey1));
    BOOST_CHECK(!key2C.VerifyPubKey(pubkey1C));
    BOOST_CHECK(!key2C.VerifyPubKey(pubkey2));
    BOOST_CHECK(key2C.VerifyPubKey(pubkey2C));

    BOOST_CHECK(DecodeDestination(addr1)  == CTxDestination(pubkey1.GetID()));
    BOOST_CHECK(DecodeDestination(addr2)  == CTxDestination(pubkey2.GetID()));
    BOOST_CHECK(DecodeDestination(addr1C) == CTxDestination(pubkey1C.GetID()));
    BOOST_CHECK(DecodeDestination(addr2C) == CTxDestination(pubkey2C.GetID()));

    for (int n=0; n<16; n++)
    {
        std::string strMsg = strprintf("Very secret message %i: 11", n);
        uint256 hashMsg = Hash(strMsg.begin(), strMsg.end());

        // normal signatures

        std::vector<unsigned char> sign1, sign2, sign1C, sign2C;

        BOOST_CHECK(key1.Sign (hashMsg, sign1));
        BOOST_CHECK(key2.Sign (hashMsg, sign2));
        BOOST_CHECK(key1C.Sign(hashMsg, sign1C));
        BOOST_CHECK(key2C.Sign(hashMsg, sign2C));

        BOOST_CHECK( pubkey1.Verify(hashMsg, sign1));
        BOOST_CHECK(!pubkey1.Verify(hashMsg, sign2));
        BOOST_CHECK( pubkey1.Verify(hashMsg, sign1C));
        BOOST_CHECK(!pubkey1.Verify(hashMsg, sign2C));

        BOOST_CHECK(!pubkey2.Verify(hashMsg, sign1));
        BOOST_CHECK( pubkey2.Verify(hashMsg, sign2));
        BOOST_CHECK(!pubkey2.Verify(hashMsg, sign1C));
        BOOST_CHECK( pubkey2.Verify(hashMsg, sign2C));

        BOOST_CHECK( pubkey1C.Verify(hashMsg, sign1));
        BOOST_CHECK(!pubkey1C.Verify(hashMsg, sign2));
        BOOST_CHECK( pubkey1C.Verify(hashMsg, sign1C));
        BOOST_CHECK(!pubkey1C.Verify(hashMsg, sign2C));

        BOOST_CHECK(!pubkey2C.Verify(hashMsg, sign1));
        BOOST_CHECK( pubkey2C.Verify(hashMsg, sign2));
        BOOST_CHECK(!pubkey2C.Verify(hashMsg, sign1C));
        BOOST_CHECK( pubkey2C.Verify(hashMsg, sign2C));

        // compact signatures (with key recovery)

        std::vector<unsigned char> csign1, csign2, csign1C, csign2C;

        BOOST_CHECK(key1.SignCompact (hashMsg, csign1));
        BOOST_CHECK(key2.SignCompact (hashMsg, csign2));
        BOOST_CHECK(key1C.SignCompact(hashMsg, csign1C));
        BOOST_CHECK(key2C.SignCompact(hashMsg, csign2C));

        CPubKey rkey1, rkey2, rkey1C, rkey2C;

        BOOST_CHECK(rkey1.RecoverCompact (hashMsg, csign1));
        BOOST_CHECK(rkey2.RecoverCompact (hashMsg, csign2));
        BOOST_CHECK(rkey1C.RecoverCompact(hashMsg, csign1C));
        BOOST_CHECK(rkey2C.RecoverCompact(hashMsg, csign2C));

        BOOST_CHECK(rkey1  == pubkey1);
        BOOST_CHECK(rkey2  == pubkey2);
        BOOST_CHECK(rkey1C == pubkey1C);
        BOOST_CHECK(rkey2C == pubkey2C);
    }

    // test deterministic signing

    std::vector<unsigned char> detsig, detsigc;
    std::string strMsg = "Very deterministic message";
    uint256 hashMsg = Hash(strMsg.begin(), strMsg.end());
    BOOST_CHECK(key1.Sign(hashMsg, detsig));
    BOOST_CHECK(key1C.Sign(hashMsg, detsigc));
    BOOST_CHECK(detsig == detsigc);
    //std::cerr << "\n" << HexStr(detsig) << "\n"; 
    BOOST_CHECK(detsig == ParseHex("3045022100e5b1ad2350d415cfc3d13025e5769e03f27117e62ab49eff991a8da141a25b7c02202e51cad7a2ea23a12618f4cba961aa10af432da6b62508a284819fe11ffe8b72"));
    BOOST_CHECK(key2.Sign(hashMsg, detsig));
    BOOST_CHECK(key2C.Sign(hashMsg, detsigc));
    BOOST_CHECK(detsig == detsigc);
    //std::cerr << "\n" << HexStr(detsig) << "\n"; 
    BOOST_CHECK(detsig == ParseHex("3044022079c109267004181a75a16c24d75ee181f1a8045eacb3dc7ab1085c98fbdb57f80220360247cc6af54f18b6ed5c68709028628c93035cc31818132fdcfb3da2a7029b"));
    BOOST_CHECK(key1.SignCompact(hashMsg, detsig));
    BOOST_CHECK(key1C.SignCompact(hashMsg, detsigc));
    //std::cerr << "\n" << HexStr(detsig) << "\n"; 
    BOOST_CHECK(detsig == ParseHex("1ce5b1ad2350d415cfc3d13025e5769e03f27117e62ab49eff991a8da141a25b7c2e51cad7a2ea23a12618f4cba961aa10af432da6b62508a284819fe11ffe8b72"));
    //std::cerr << "\n" << HexStr(detsigc) << "\n"; 
    BOOST_CHECK(detsigc == ParseHex("20e5b1ad2350d415cfc3d13025e5769e03f27117e62ab49eff991a8da141a25b7c2e51cad7a2ea23a12618f4cba961aa10af432da6b62508a284819fe11ffe8b72"));
    BOOST_CHECK(key2.SignCompact(hashMsg, detsig));
    BOOST_CHECK(key2C.SignCompact(hashMsg, detsigc));
    //std::cerr << "\n" << HexStr(detsig) << "\n"; 
    BOOST_CHECK(detsig == ParseHex("1c79c109267004181a75a16c24d75ee181f1a8045eacb3dc7ab1085c98fbdb57f8360247cc6af54f18b6ed5c68709028628c93035cc31818132fdcfb3da2a7029b"));
    //std::cerr << "\n" << HexStr(detsigc) << "\n"; 
    BOOST_CHECK(detsigc == ParseHex("2079c109267004181a75a16c24d75ee181f1a8045eacb3dc7ab1085c98fbdb57f8360247cc6af54f18b6ed5c68709028628c93035cc31818132fdcfb3da2a7029b"));
}

BOOST_AUTO_TEST_SUITE_END()
