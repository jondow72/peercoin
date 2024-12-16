// Copyright (c) 2012-2022 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <core_io.h>
#include <interfaces/chain.h>
#include <node/context.h>
#include <rpc/blockchain.h>
#include <rpc/client.h>
#include <rpc/server.h>
#include <rpc/util.h>
#include <test/util/setup_common.h>
#include <univalue.h>
#include <util/time.h>

#include <any>

#include <boost/test/unit_test.hpp>

static UniValue JSON(std::string_view json)
{
    UniValue value;
    BOOST_CHECK(value.read(json.data(), json.size()));
    return value;
}

class HasJSON
{
public:
    explicit HasJSON(std::string json) : m_json(std::move(json)) {}
    bool operator()(const UniValue& value) const
    {
        std::string json{value.write()};
        BOOST_CHECK_EQUAL(json, m_json);
        return json == m_json;
    };

private:
    const std::string m_json;
};

class RPCTestingSetup : public TestingSetup
{
public:
    UniValue TransformParams(const UniValue& params, std::vector<std::string> arg_names) const;
    UniValue CallRPC(std::string args);
};

UniValue RPCTestingSetup::TransformParams(const UniValue& params, std::vector<std::string> arg_names) const
{
    UniValue transformed_params;
    CRPCTable table;
    CRPCCommand command{"category", "method", [&](const JSONRPCRequest& request, UniValue&, bool) -> bool { transformed_params = request.params; return true; }, arg_names, /*unique_id=*/0};
    table.appendCommand("method", &command);
    JSONRPCRequest request;
    request.strMethod = "method";
    request.params = params;
    if (RPCIsInWarmup(nullptr)) SetRPCWarmupFinished();
    table.execute(request);
    return transformed_params;
}

UniValue RPCTestingSetup::CallRPC(std::string args)
{
    std::vector<std::string> vArgs{SplitString(args, ' ')};
    std::string strMethod = vArgs[0];
    vArgs.erase(vArgs.begin());
    JSONRPCRequest request;
    request.context = &m_node;
    request.strMethod = strMethod;
    request.params = RPCConvertValues(strMethod, vArgs);
    if (RPCIsInWarmup(nullptr)) SetRPCWarmupFinished();
    try {
        UniValue result = tableRPC.execute(request);
        return result;
    }
    catch (const UniValue& objError) {
        throw std::runtime_error(find_value(objError, "message").get_str());
    }
}


BOOST_FIXTURE_TEST_SUITE(rpc_tests, RPCTestingSetup)

BOOST_AUTO_TEST_CASE(rpc_namedparams)
{
    const std::vector<std::string> arg_names{"arg1", "arg2", "arg3", "arg4", "arg5"};

    // Make sure named arguments are transformed into positional arguments in correct places separated by nulls
    BOOST_CHECK_EQUAL(TransformParams(JSON(R"({"arg2": 2, "arg4": 4})"), arg_names).write(), "[null,2,null,4]");

    // Make sure named argument specified multiple times raises an exception
    BOOST_CHECK_EXCEPTION(TransformParams(JSON(R"({"arg2": 2, "arg2": 4})"), arg_names), UniValue,
                          HasJSON(R"({"code":-8,"message":"Parameter arg2 specified multiple times"})"));

    // Make sure named and positional arguments can be combined.
    BOOST_CHECK_EQUAL(TransformParams(JSON(R"({"arg5": 5, "args": [1, 2], "arg4": 4})"), arg_names).write(), "[1,2,null,4,5]");

    // Make sure a unknown named argument raises an exception
    BOOST_CHECK_EXCEPTION(TransformParams(JSON(R"({"arg2": 2, "unknown": 6})"), arg_names), UniValue,
                          HasJSON(R"({"code":-8,"message":"Unknown named parameter unknown"})"));

    // Make sure an overlap between a named argument and positional argument raises an exception
    BOOST_CHECK_EXCEPTION(TransformParams(JSON(R"({"args": [1,2,3], "arg4": 4, "arg2": 2})"), arg_names), UniValue,
                          HasJSON(R"({"code":-8,"message":"Parameter arg2 specified twice both as positional and named argument"})"));

    // Make sure extra positional arguments can be passed through to the method implementation, as long as they don't overlap with named arguments.
    BOOST_CHECK_EQUAL(TransformParams(JSON(R"({"args": [1,2,3,4,5,6,7,8,9,10]})"), arg_names).write(), "[1,2,3,4,5,6,7,8,9,10]");
    BOOST_CHECK_EQUAL(TransformParams(JSON(R"([1,2,3,4,5,6,7,8,9,10])"), arg_names).write(), "[1,2,3,4,5,6,7,8,9,10]");
}

BOOST_AUTO_TEST_CASE(rpc_rawparams)
{
    // Test raw transaction API argument handling
    UniValue r;

    BOOST_CHECK_THROW(CallRPC("getrawtransaction"), std::runtime_error);
    BOOST_CHECK_THROW(CallRPC("getrawtransaction not_hex"), std::runtime_error);
    BOOST_CHECK_THROW(CallRPC("getrawtransaction a3b807410df0b60fcb9736768df5823938b2f838694939ba45f3c0a1bff150ed not_int"), std::runtime_error);

    BOOST_CHECK_THROW(CallRPC("createrawtransaction"), std::runtime_error);
    BOOST_CHECK_THROW(CallRPC("createrawtransaction null null"), std::runtime_error);
    BOOST_CHECK_THROW(CallRPC("createrawtransaction not_array"), std::runtime_error);
    BOOST_CHECK_THROW(CallRPC("createrawtransaction {} {}"), std::runtime_error);
    BOOST_CHECK_NO_THROW(CallRPC("createrawtransaction [] {}"));
    BOOST_CHECK_THROW(CallRPC("createrawtransaction [] {} extra"), std::runtime_error);

    BOOST_CHECK_THROW(CallRPC("decoderawtransaction"), std::runtime_error);
    BOOST_CHECK_THROW(CallRPC("decoderawtransaction null"), std::runtime_error);
    BOOST_CHECK_THROW(CallRPC("decoderawtransaction DEADBEEF"), std::runtime_error);
    std::string rawtx = "010000001209a35e0150afd8cc27e9f6bdfdda98bdcb5cf9ffe82b479bb969e908ff0e2357ecd765c00100000048473044022077a33181fed749626ba02d41db813f53e61be4ad0b8d856fecda5977932559300220260106f50d83b82368192ae4ac4c3697951449bff18d266e25356a6d91e97de701ffffffff0300000000000000000008287e010000000023210327f1f1fc8fbd47411ab995879dbdc9f6db8f41a762ee86d028a0ca063e36b175acc82b7e010000000023210327f1f1fc8fbd47411ab995879dbdc9f6db8f41a762ee86d028a0ca063e36b175ac00000000";
    BOOST_CHECK_NO_THROW(r = CallRPC(std::string("decoderawtransaction ")+rawtx));
    BOOST_CHECK_EQUAL(find_value(r.get_obj(), "size").getInt<int>(), 224);
    BOOST_CHECK_EQUAL(find_value(r.get_obj(), "version").getInt<int>(), 1);
    BOOST_CHECK_EQUAL(find_value(r.get_obj(), "locktime").getInt<int>(), 0);
    BOOST_CHECK_THROW(CallRPC(std::string("decoderawtransaction ")+rawtx+" extra"), std::runtime_error);
    BOOST_CHECK_NO_THROW(r = CallRPC(std::string("decoderawtransaction ")+rawtx+" false"));
    BOOST_CHECK_THROW(r = CallRPC(std::string("decoderawtransaction ")+rawtx+" false extra"), std::runtime_error);

    // Only check failure cases for sendrawtransaction, there's no network to send to...
    BOOST_CHECK_THROW(CallRPC("sendrawtransaction"), std::runtime_error);
    BOOST_CHECK_THROW(CallRPC("sendrawtransaction null"), std::runtime_error);
    BOOST_CHECK_THROW(CallRPC("sendrawtransaction DEADBEEF"), std::runtime_error);
    BOOST_CHECK_THROW(CallRPC(std::string("sendrawtransaction ")+rawtx+" extra"), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(rpc_togglenetwork)
{
    UniValue r;

    r = CallRPC("getnetworkinfo");
    bool netState = find_value(r.get_obj(), "networkactive").get_bool();
    BOOST_CHECK_EQUAL(netState, true);

    BOOST_CHECK_NO_THROW(CallRPC("setnetworkactive false"));
    r = CallRPC("getnetworkinfo");
    int numConnection = find_value(r.get_obj(), "connections").getInt<int>();
    BOOST_CHECK_EQUAL(numConnection, 0);

    netState = find_value(r.get_obj(), "networkactive").get_bool();
    BOOST_CHECK_EQUAL(netState, false);

    BOOST_CHECK_NO_THROW(CallRPC("setnetworkactive true"));
    r = CallRPC("getnetworkinfo");
    netState = find_value(r.get_obj(), "networkactive").get_bool();
    BOOST_CHECK_EQUAL(netState, true);
}

BOOST_AUTO_TEST_CASE(rpc_rawsign)
{
    UniValue r;
    // input is a 1-of-2 multisig (so is output):
    std::string prevout =
      "[{\"txid\":\"b4cc287e58f87cdae59417329f710f3ecd75a4ee1d2872b7248f50977c8493f3\","
      "\"vout\":1,\"scriptPubKey\":\"a914809941f7fdb9675963477e0da14c66390a84ccac87\","
      "\"redeemScript\":\"5221038024dd4e955f93fa4ba5a9d7c9f01d2419fe6d7ae0d2aaaed6fe32aff0b8ebfa2103ba8d94305d366d30d5a41a658b5e2859d00d43c870dc0a3cdda7e52ddb4ff20652ae\"}]";
    r = CallRPC(std::string("createrawtransaction ")+prevout+" "+
      "{\"PKhs4P7KEUv1GCUbcxYEx1fEBFc7yyfsm9\":11}");
    std::string notsigned = r.get_str();
    std::string privkey1 = "\"U9MHK7o3WQbCD4kNJAye1PtHDKBCn9LU2BGwQJaqcsQqPcDAVSYc\"";
    std::string privkey2 = "\"UA6rEKCCjpYG4V6AQfcd9V7ZZZzcHBT1M5CtTyq3jwnQNgrdkbd1\"";
    r = CallRPC(std::string("signrawtransactionwithkey ")+notsigned+" [] "+prevout);
    BOOST_CHECK(find_value(r.get_obj(), "complete").get_bool() == false);
    r = CallRPC(std::string("signrawtransactionwithkey ")+notsigned+" ["+privkey1+","+privkey2+"] "+prevout);
    BOOST_CHECK(find_value(r.get_obj(), "complete").get_bool() == true);
}

BOOST_AUTO_TEST_CASE(rpc_createraw_op_return)
{
    BOOST_CHECK_NO_THROW(CallRPC("createrawtransaction [{\"txid\":\"a3b807410df0b60fcb9736768df5823938b2f838694939ba45f3c0a1bff150ed\",\"vout\":0}] {\"data\":\"68656c6c6f776f726c64\"}"));

    // Key not "data" (bad address)
    BOOST_CHECK_THROW(CallRPC("createrawtransaction [{\"txid\":\"a3b807410df0b60fcb9736768df5823938b2f838694939ba45f3c0a1bff150ed\",\"vout\":0}] {\"somedata\":\"68656c6c6f776f726c64\"}"), std::runtime_error);

    // Bad hex encoding of data output
    BOOST_CHECK_THROW(CallRPC("createrawtransaction [{\"txid\":\"a3b807410df0b60fcb9736768df5823938b2f838694939ba45f3c0a1bff150ed\",\"vout\":0}] {\"data\":\"12345\"}"), std::runtime_error);
    BOOST_CHECK_THROW(CallRPC("createrawtransaction [{\"txid\":\"a3b807410df0b60fcb9736768df5823938b2f838694939ba45f3c0a1bff150ed\",\"vout\":0}] {\"data\":\"12345g\"}"), std::runtime_error);

    // Data 81 bytes long
    BOOST_CHECK_NO_THROW(CallRPC("createrawtransaction [{\"txid\":\"a3b807410df0b60fcb9736768df5823938b2f838694939ba45f3c0a1bff150ed\",\"vout\":0}] {\"data\":\"010203040506070809101112131415161718192021222324252627282930313233343536373839404142434445464748495051525354555657585960616263646566676869707172737475767778798081\"}"));
}

BOOST_AUTO_TEST_CASE(rpc_format_monetary_values)
{
    BOOST_CHECK(ValueFromAmount(0LL).write() == "0.000000");
    BOOST_CHECK(ValueFromAmount(1LL).write() == "0.000001");
    BOOST_CHECK(ValueFromAmount(176221LL).write() == "0.176221");
    BOOST_CHECK(ValueFromAmount(500000LL).write() == "0.500000");
    BOOST_CHECK(ValueFromAmount(898989LL).write() == "0.898989");
    BOOST_CHECK(ValueFromAmount(1000000LL).write() == "1.000000");
    BOOST_CHECK(ValueFromAmount(20999999999990LL).write() == "20999999.999990");
    BOOST_CHECK(ValueFromAmount(20999999999999LL).write() == "20999999.999999");

    BOOST_CHECK_EQUAL(ValueFromAmount(0).write(), "0.000000");
    BOOST_CHECK_EQUAL(ValueFromAmount((COIN/10000)*123456789).write(), "12345.678900");
    BOOST_CHECK_EQUAL(ValueFromAmount(-COIN).write(), "-1.000000");
    BOOST_CHECK_EQUAL(ValueFromAmount(-COIN/10).write(), "-0.100000");

    BOOST_CHECK_EQUAL(ValueFromAmount(COIN*100000000).write(), "100000000.000000");
    BOOST_CHECK_EQUAL(ValueFromAmount(COIN*10000000).write(), "10000000.000000");
    BOOST_CHECK_EQUAL(ValueFromAmount(COIN*1000000).write(), "1000000.000000");
    BOOST_CHECK_EQUAL(ValueFromAmount(COIN*100000).write(), "100000.000000");
    BOOST_CHECK_EQUAL(ValueFromAmount(COIN*10000).write(), "10000.000000");
    BOOST_CHECK_EQUAL(ValueFromAmount(COIN*1000).write(), "1000.000000");
    BOOST_CHECK_EQUAL(ValueFromAmount(COIN*100).write(), "100.000000");
    BOOST_CHECK_EQUAL(ValueFromAmount(COIN*10).write(), "10.000000");
    BOOST_CHECK_EQUAL(ValueFromAmount(COIN).write(), "1.000000");
    BOOST_CHECK_EQUAL(ValueFromAmount(COIN/10).write(), "0.100000");
    BOOST_CHECK_EQUAL(ValueFromAmount(COIN/100).write(), "0.010000");
    BOOST_CHECK_EQUAL(ValueFromAmount(COIN/1000).write(), "0.001000");
    BOOST_CHECK_EQUAL(ValueFromAmount(COIN/10000).write(), "0.000100");
    BOOST_CHECK_EQUAL(ValueFromAmount(COIN/100000).write(), "0.000010");
    BOOST_CHECK_EQUAL(ValueFromAmount(COIN/1000000).write(), "0.000001");

    BOOST_CHECK_EQUAL(ValueFromAmount(std::numeric_limits<CAmount>::max()).write(), "9223372036854.775807");
    BOOST_CHECK_EQUAL(ValueFromAmount(std::numeric_limits<CAmount>::max() - 1).write(), "9223372036854.775806");
    BOOST_CHECK_EQUAL(ValueFromAmount(std::numeric_limits<CAmount>::max() - 2).write(), "9223372036854.775805");
    BOOST_CHECK_EQUAL(ValueFromAmount(std::numeric_limits<CAmount>::max() - 3).write(), "9223372036854.775804");
    // ...
    BOOST_CHECK_EQUAL(ValueFromAmount(std::numeric_limits<CAmount>::min() + 3).write(), "-9223372036854.775805");
    BOOST_CHECK_EQUAL(ValueFromAmount(std::numeric_limits<CAmount>::min() + 2).write(), "-9223372036854.775806");
    BOOST_CHECK_EQUAL(ValueFromAmount(std::numeric_limits<CAmount>::min() + 1).write(), "-9223372036854.775807");
    BOOST_CHECK_EQUAL(ValueFromAmount(std::numeric_limits<CAmount>::min()).write(), "-9223372036854.775808");
}

static UniValue ValueFromString(const std::string& str) noexcept
{
    UniValue value;
    value.setNumStr(str);
    return value;
}

BOOST_AUTO_TEST_CASE(rpc_parse_monetary_values)
{
    BOOST_CHECK_THROW(AmountFromValue(ValueFromString("-0.000001")), UniValue);
    BOOST_CHECK_EQUAL(AmountFromValue(ValueFromString("0")), 0LL);
    BOOST_CHECK_EQUAL(AmountFromValue(ValueFromString("0.000000")), 0LL);
    BOOST_CHECK_EQUAL(AmountFromValue(ValueFromString("0.000001")), 1LL);
    BOOST_CHECK_EQUAL(AmountFromValue(ValueFromString("0.176221")), 176221LL);
    BOOST_CHECK_EQUAL(AmountFromValue(ValueFromString("0.5")), 500000LL);
    BOOST_CHECK_EQUAL(AmountFromValue(ValueFromString("0.500000")), 500000LL);
    BOOST_CHECK_EQUAL(AmountFromValue(ValueFromString("0.898989")), 898989LL);
    BOOST_CHECK_EQUAL(AmountFromValue(ValueFromString("1.000000")), 1000000LL);
    BOOST_CHECK_EQUAL(AmountFromValue(ValueFromString("20999999.99999")), 20999999999990LL);
    BOOST_CHECK_EQUAL(AmountFromValue(ValueFromString("20999999.999999")), 20999999999999LL);

    // check that fix 37df99c3856c1d9e81d895425ea395311d42dc1d works
    BOOST_CHECK_EQUAL(AmountFromValue(ValueFromString("0.00000111")), 1LL);
    BOOST_CHECK_EQUAL(AmountFromValue(ValueFromString("0.00000199")), 1LL);
    BOOST_CHECK_THROW(AmountFromValue(ValueFromString("0.000001009")), UniValue);

    BOOST_CHECK_EQUAL(AmountFromValue(ValueFromString("1e-6")), COIN/1000000);
    BOOST_CHECK_EQUAL(AmountFromValue(ValueFromString("0.1e-5")), COIN/1000000);
    BOOST_CHECK_EQUAL(AmountFromValue(ValueFromString("0.01e-4")), COIN/1000000);
    BOOST_CHECK_EQUAL(AmountFromValue(ValueFromString("0.00000000000000000000000000000000000000000000000000000000000000000000000001e+68")), COIN/1000000);
    BOOST_CHECK_EQUAL(AmountFromValue(ValueFromString("10000000000000000000000000000000000000000000000000000000000000000e-64")), COIN);
    BOOST_CHECK_EQUAL(AmountFromValue(ValueFromString("0.000000000000000000000000000000000000000000000000000000000000000100000000000000000000000000000000000000000000000000000e64")), COIN);

    BOOST_CHECK_THROW(AmountFromValue(ValueFromString("1e-9")), UniValue); //should fail
    BOOST_CHECK_THROW(AmountFromValue(ValueFromString("0.000000019")), UniValue); //should fail
    BOOST_CHECK_EQUAL(AmountFromValue(ValueFromString("0.000001000000")), 1LL); //should pass, cut trailing 0
    BOOST_CHECK_THROW(AmountFromValue(ValueFromString("19e-9")), UniValue); //should fail
    BOOST_CHECK_EQUAL(AmountFromValue(ValueFromString("0.19e-4")), 19); //should pass, leading 0 is present

    BOOST_CHECK_THROW(AmountFromValue(ValueFromString("92233720368.54775808")), UniValue); //overflow error
    BOOST_CHECK_THROW(AmountFromValue(ValueFromString("1e+11")), UniValue); //overflow error
    BOOST_CHECK_THROW(AmountFromValue(ValueFromString("1e11")), UniValue); //overflow error signless
    BOOST_CHECK_THROW(AmountFromValue(ValueFromString("93e+9")), UniValue); //overflow error
}

BOOST_AUTO_TEST_CASE(json_parse_errors)
{
    // Valid
    BOOST_CHECK_EQUAL(ParseNonRFCJSONValue("1.0").get_real(), 1.0);
    BOOST_CHECK_EQUAL(ParseNonRFCJSONValue("true").get_bool(), true);
    BOOST_CHECK_EQUAL(ParseNonRFCJSONValue("[false]")[0].get_bool(), false);
    BOOST_CHECK_EQUAL(ParseNonRFCJSONValue("{\"a\": true}")["a"].get_bool(), true);
    BOOST_CHECK_EQUAL(ParseNonRFCJSONValue("{\"1\": \"true\"}")["1"].get_str(), "true");
    // Valid, with leading or trailing whitespace
    BOOST_CHECK_EQUAL(ParseNonRFCJSONValue(" 1.0").get_real(), 1.0);
    BOOST_CHECK_EQUAL(ParseNonRFCJSONValue("1.0 ").get_real(), 1.0);

    BOOST_CHECK_THROW(AmountFromValue(ParseNonRFCJSONValue(".19e-6")), std::runtime_error); //should fail, missing leading 0, therefore invalid JSON
    BOOST_CHECK_EQUAL(AmountFromValue(ParseNonRFCJSONValue("0.000000000000000000000000000000000001e+30 ")), 1);

    // Invalid, initial garbage
    BOOST_CHECK_THROW(ParseNonRFCJSONValue("[1.0"), std::runtime_error);
    BOOST_CHECK_THROW(ParseNonRFCJSONValue("a1.0"), std::runtime_error);
    // Invalid, trailing garbage
    BOOST_CHECK_THROW(ParseNonRFCJSONValue("1.0sds"), std::runtime_error);
    BOOST_CHECK_THROW(ParseNonRFCJSONValue("1.0]"), std::runtime_error);
    // Invalid, keys have to be names
    BOOST_CHECK_THROW(ParseNonRFCJSONValue("{1: \"true\"}"), std::runtime_error);
    BOOST_CHECK_THROW(ParseNonRFCJSONValue("{true: 1}"), std::runtime_error);
    BOOST_CHECK_THROW(ParseNonRFCJSONValue("{[1]: 1}"), std::runtime_error);
    BOOST_CHECK_THROW(ParseNonRFCJSONValue("{{\"a\": \"a\"}: 1}"), std::runtime_error);
    // BTC addresses should fail parsing
    BOOST_CHECK_THROW(ParseNonRFCJSONValue("175tWpb8K1S7NmH4Zx6rewF9WQrcZv245W"), std::runtime_error);
    BOOST_CHECK_THROW(ParseNonRFCJSONValue("3J98t1WpEZ73CNmQviecrnyiWrnqRhWNL"), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(rpc_ban)
{
    BOOST_CHECK_NO_THROW(CallRPC(std::string("clearbanned")));

    UniValue r;
    BOOST_CHECK_NO_THROW(r = CallRPC(std::string("setban 127.0.0.0 add")));
    BOOST_CHECK_THROW(r = CallRPC(std::string("setban 127.0.0.0:8334")), std::runtime_error); //portnumber for setban not allowed
    BOOST_CHECK_NO_THROW(r = CallRPC(std::string("listbanned")));
    UniValue ar = r.get_array();
    UniValue o1 = ar[0].get_obj();
    UniValue adr = find_value(o1, "address");
    BOOST_CHECK_EQUAL(adr.get_str(), "127.0.0.0/32");
    BOOST_CHECK_NO_THROW(CallRPC(std::string("setban 127.0.0.0 remove")));
    BOOST_CHECK_NO_THROW(r = CallRPC(std::string("listbanned")));
    ar = r.get_array();
    BOOST_CHECK_EQUAL(ar.size(), 0U);

    BOOST_CHECK_NO_THROW(r = CallRPC(std::string("setban 127.0.0.0/24 add 9907731200 true")));
    BOOST_CHECK_NO_THROW(r = CallRPC(std::string("listbanned")));
    ar = r.get_array();
    o1 = ar[0].get_obj();
    adr = find_value(o1, "address");
    int64_t banned_until{find_value(o1, "banned_until").getInt<int64_t>()};
    BOOST_CHECK_EQUAL(adr.get_str(), "127.0.0.0/24");
    BOOST_CHECK_EQUAL(banned_until, 9907731200); // absolute time check

    BOOST_CHECK_NO_THROW(CallRPC(std::string("clearbanned")));

    auto now = 10'000s;
    SetMockTime(now);
    BOOST_CHECK_NO_THROW(r = CallRPC(std::string("setban 127.0.0.0/24 add 200")));
    SetMockTime(now += 2s);
    const int64_t time_remaining_expected{198};
    BOOST_CHECK_NO_THROW(r = CallRPC(std::string("listbanned")));
    ar = r.get_array();
    o1 = ar[0].get_obj();
    adr = find_value(o1, "address");
    banned_until = find_value(o1, "banned_until").getInt<int64_t>();
    const int64_t ban_created{find_value(o1, "ban_created").getInt<int64_t>()};
    const int64_t ban_duration{find_value(o1, "ban_duration").getInt<int64_t>()};
    const int64_t time_remaining{find_value(o1, "time_remaining").getInt<int64_t>()};
    BOOST_CHECK_EQUAL(adr.get_str(), "127.0.0.0/24");
    BOOST_CHECK_EQUAL(banned_until, time_remaining_expected + now.count());
    BOOST_CHECK_EQUAL(ban_duration, banned_until - ban_created);
    BOOST_CHECK_EQUAL(time_remaining, time_remaining_expected);

    // must throw an exception because 127.0.0.1 is in already banned subnet range
    BOOST_CHECK_THROW(r = CallRPC(std::string("setban 127.0.0.1 add")), std::runtime_error);

    BOOST_CHECK_NO_THROW(CallRPC(std::string("setban 127.0.0.0/24 remove")));
    BOOST_CHECK_NO_THROW(r = CallRPC(std::string("listbanned")));
    ar = r.get_array();
    BOOST_CHECK_EQUAL(ar.size(), 0U);

    BOOST_CHECK_NO_THROW(r = CallRPC(std::string("setban 127.0.0.0/255.255.0.0 add")));
    BOOST_CHECK_THROW(r = CallRPC(std::string("setban 127.0.1.1 add")), std::runtime_error);

    BOOST_CHECK_NO_THROW(CallRPC(std::string("clearbanned")));
    BOOST_CHECK_NO_THROW(r = CallRPC(std::string("listbanned")));
    ar = r.get_array();
    BOOST_CHECK_EQUAL(ar.size(), 0U);


    BOOST_CHECK_THROW(r = CallRPC(std::string("setban test add")), std::runtime_error); //invalid IP

    //IPv6 tests
    BOOST_CHECK_NO_THROW(r = CallRPC(std::string("setban FE80:0000:0000:0000:0202:B3FF:FE1E:8329 add")));
    BOOST_CHECK_NO_THROW(r = CallRPC(std::string("listbanned")));
    ar = r.get_array();
    o1 = ar[0].get_obj();
    adr = find_value(o1, "address");
    BOOST_CHECK_EQUAL(adr.get_str(), "fe80::202:b3ff:fe1e:8329/128");

    BOOST_CHECK_NO_THROW(CallRPC(std::string("clearbanned")));
    BOOST_CHECK_NO_THROW(r = CallRPC(std::string("setban 2001:db8::/ffff:fffc:0:0:0:0:0:0 add")));
    BOOST_CHECK_NO_THROW(r = CallRPC(std::string("listbanned")));
    ar = r.get_array();
    o1 = ar[0].get_obj();
    adr = find_value(o1, "address");
    BOOST_CHECK_EQUAL(adr.get_str(), "2001:db8::/30");

    BOOST_CHECK_NO_THROW(CallRPC(std::string("clearbanned")));
    BOOST_CHECK_NO_THROW(r = CallRPC(std::string("setban 2001:4d48:ac57:400:cacf:e9ff:fe1d:9c63/128 add")));
    BOOST_CHECK_NO_THROW(r = CallRPC(std::string("listbanned")));
    ar = r.get_array();
    o1 = ar[0].get_obj();
    adr = find_value(o1, "address");
    BOOST_CHECK_EQUAL(adr.get_str(), "2001:4d48:ac57:400:cacf:e9ff:fe1d:9c63/128");
}

BOOST_AUTO_TEST_CASE(rpc_convert_values_generatetoaddress)
{
    UniValue result;

    BOOST_CHECK_NO_THROW(result = RPCConvertValues("generatetoaddress", {"101", "mkESjLZW66TmHhiFX8MCaBjrhZ543PPh9a"}));
    BOOST_CHECK_EQUAL(result[0].getInt<int>(), 101);
    BOOST_CHECK_EQUAL(result[1].get_str(), "mkESjLZW66TmHhiFX8MCaBjrhZ543PPh9a");

    BOOST_CHECK_NO_THROW(result = RPCConvertValues("generatetoaddress", {"101", "mhMbmE2tE9xzJYCV9aNC8jKWN31vtGrguU"}));
    BOOST_CHECK_EQUAL(result[0].getInt<int>(), 101);
    BOOST_CHECK_EQUAL(result[1].get_str(), "mhMbmE2tE9xzJYCV9aNC8jKWN31vtGrguU");

    BOOST_CHECK_NO_THROW(result = RPCConvertValues("generatetoaddress", {"1", "mkESjLZW66TmHhiFX8MCaBjrhZ543PPh9a", "9"}));
    BOOST_CHECK_EQUAL(result[0].getInt<int>(), 1);
    BOOST_CHECK_EQUAL(result[1].get_str(), "mkESjLZW66TmHhiFX8MCaBjrhZ543PPh9a");
    BOOST_CHECK_EQUAL(result[2].getInt<int>(), 9);

    BOOST_CHECK_NO_THROW(result = RPCConvertValues("generatetoaddress", {"1", "mhMbmE2tE9xzJYCV9aNC8jKWN31vtGrguU", "9"}));
    BOOST_CHECK_EQUAL(result[0].getInt<int>(), 1);
    BOOST_CHECK_EQUAL(result[1].get_str(), "mhMbmE2tE9xzJYCV9aNC8jKWN31vtGrguU");
    BOOST_CHECK_EQUAL(result[2].getInt<int>(), 9);
}

BOOST_AUTO_TEST_CASE(rpc_getblockstats_calculate_percentiles_by_weight)
{
    int64_t total_weight = 200;
    std::vector<std::pair<CAmount, int64_t>> feerates;
    CAmount result[NUM_GETBLOCKSTATS_PERCENTILES] = { 0 };

    for (int64_t i = 0; i < 100; i++) {
        feerates.emplace_back(std::make_pair(1 ,1));
    }

    for (int64_t i = 0; i < 100; i++) {
        feerates.emplace_back(std::make_pair(2 ,1));
    }

    CalculatePercentilesByWeight(result, feerates, total_weight);
    BOOST_CHECK_EQUAL(result[0], 1);
    BOOST_CHECK_EQUAL(result[1], 1);
    BOOST_CHECK_EQUAL(result[2], 1);
    BOOST_CHECK_EQUAL(result[3], 2);
    BOOST_CHECK_EQUAL(result[4], 2);

    // Test with more pairs, and two pairs overlapping 2 percentiles.
    total_weight = 100;
    CAmount result2[NUM_GETBLOCKSTATS_PERCENTILES] = { 0 };
    feerates.clear();

    feerates.emplace_back(std::make_pair(1, 9));
    feerates.emplace_back(std::make_pair(2 , 16)); //10th + 25th percentile
    feerates.emplace_back(std::make_pair(4 ,50)); //50th + 75th percentile
    feerates.emplace_back(std::make_pair(5 ,10));
    feerates.emplace_back(std::make_pair(9 ,15));  // 90th percentile

    CalculatePercentilesByWeight(result2, feerates, total_weight);

    BOOST_CHECK_EQUAL(result2[0], 2);
    BOOST_CHECK_EQUAL(result2[1], 2);
    BOOST_CHECK_EQUAL(result2[2], 4);
    BOOST_CHECK_EQUAL(result2[3], 4);
    BOOST_CHECK_EQUAL(result2[4], 9);

    // Same test as above, but one of the percentile-overlapping pairs is split in 2.
    total_weight = 100;
    CAmount result3[NUM_GETBLOCKSTATS_PERCENTILES] = { 0 };
    feerates.clear();

    feerates.emplace_back(std::make_pair(1, 9));
    feerates.emplace_back(std::make_pair(2 , 11)); // 10th percentile
    feerates.emplace_back(std::make_pair(2 , 5)); // 25th percentile
    feerates.emplace_back(std::make_pair(4 ,50)); //50th + 75th percentile
    feerates.emplace_back(std::make_pair(5 ,10));
    feerates.emplace_back(std::make_pair(9 ,15)); // 90th percentile

    CalculatePercentilesByWeight(result3, feerates, total_weight);

    BOOST_CHECK_EQUAL(result3[0], 2);
    BOOST_CHECK_EQUAL(result3[1], 2);
    BOOST_CHECK_EQUAL(result3[2], 4);
    BOOST_CHECK_EQUAL(result3[3], 4);
    BOOST_CHECK_EQUAL(result3[4], 9);

    // Test with one transaction spanning all percentiles.
    total_weight = 104;
    CAmount result4[NUM_GETBLOCKSTATS_PERCENTILES] = { 0 };
    feerates.clear();

    feerates.emplace_back(std::make_pair(1, 100));
    feerates.emplace_back(std::make_pair(2, 1));
    feerates.emplace_back(std::make_pair(3, 1));
    feerates.emplace_back(std::make_pair(3, 1));
    feerates.emplace_back(std::make_pair(999999, 1));

    CalculatePercentilesByWeight(result4, feerates, total_weight);

    for (int64_t i = 0; i < NUM_GETBLOCKSTATS_PERCENTILES; i++) {
        BOOST_CHECK_EQUAL(result4[i], 1);
    }
}

BOOST_AUTO_TEST_CASE(help_example)
{
    // test different argument types
    const RPCArgList& args = {{"foo", "bar"}, {"b", true}, {"n", 1}};
    BOOST_CHECK_EQUAL(HelpExampleCliNamed("test", args), "> bitcoin-cli -named test foo=bar b=true n=1\n");
    BOOST_CHECK_EQUAL(HelpExampleRpcNamed("test", args), "> curl --user myusername --data-binary '{\"jsonrpc\": \"1.0\", \"id\": \"curltest\", \"method\": \"test\", \"params\": {\"foo\":\"bar\",\"b\":true,\"n\":1}}' -H 'content-type: text/plain;' http://127.0.0.1:8332/\n");

    // test shell escape
    BOOST_CHECK_EQUAL(HelpExampleCliNamed("test", {{"foo", "b'ar"}}), "> bitcoin-cli -named test foo='b'''ar'\n");
    BOOST_CHECK_EQUAL(HelpExampleCliNamed("test", {{"foo", "b\"ar"}}), "> bitcoin-cli -named test foo='b\"ar'\n");
    BOOST_CHECK_EQUAL(HelpExampleCliNamed("test", {{"foo", "b ar"}}), "> bitcoin-cli -named test foo='b ar'\n");

    // test object params
    UniValue obj_value(UniValue::VOBJ);
    obj_value.pushKV("foo", "bar");
    obj_value.pushKV("b", false);
    obj_value.pushKV("n", 1);
    BOOST_CHECK_EQUAL(HelpExampleCliNamed("test", {{"name", obj_value}}), "> bitcoin-cli -named test name='{\"foo\":\"bar\",\"b\":false,\"n\":1}'\n");
    BOOST_CHECK_EQUAL(HelpExampleRpcNamed("test", {{"name", obj_value}}), "> curl --user myusername --data-binary '{\"jsonrpc\": \"1.0\", \"id\": \"curltest\", \"method\": \"test\", \"params\": {\"name\":{\"foo\":\"bar\",\"b\":false,\"n\":1}}}' -H 'content-type: text/plain;' http://127.0.0.1:8332/\n");

    // test array params
    UniValue arr_value(UniValue::VARR);
    arr_value.push_back("bar");
    arr_value.push_back(false);
    arr_value.push_back(1);
    BOOST_CHECK_EQUAL(HelpExampleCliNamed("test", {{"name", arr_value}}), "> bitcoin-cli -named test name='[\"bar\",false,1]'\n");
    BOOST_CHECK_EQUAL(HelpExampleRpcNamed("test", {{"name", arr_value}}), "> curl --user myusername --data-binary '{\"jsonrpc\": \"1.0\", \"id\": \"curltest\", \"method\": \"test\", \"params\": {\"name\":[\"bar\",false,1]}}' -H 'content-type: text/plain;' http://127.0.0.1:8332/\n");

    // test types don't matter for shell
    BOOST_CHECK_EQUAL(HelpExampleCliNamed("foo", {{"arg", true}}), HelpExampleCliNamed("foo", {{"arg", "true"}}));

    // test types matter for Rpc
    BOOST_CHECK_NE(HelpExampleRpcNamed("foo", {{"arg", true}}), HelpExampleRpcNamed("foo", {{"arg", "true"}}));
}

BOOST_AUTO_TEST_SUITE_END()
