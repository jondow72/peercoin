// Copyright (c) 2012-2025 The Peercoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <kernel.h>
#include <chainparams.h>
#include <validation.h>
#include <streams.h>
#include <timedata.h>
#include <bignum.h>
#include <txdb.h>
#include <consensus/validation.h>
#include <util/system.h>
#include <validation.h>
#include <random.h>
#include <script/interpreter.h>
#include <inttypes.h>

#include <index/txindex.h>

#include <boost/assign/list_of.hpp>
#include <../crypto/magimath.h>

using namespace std;

unsigned int nStakeMaxAge = 60 * 60 * 24 * 30;	// stake age of full weight: 30 days
static const int64_t MAX_MONEY_STAKE_REF_V2 = 500000 * COIN;	// 0.5 mil
static const double MAX_MAGI_BALANCE_in_STAKE = 0.15;		// balance/money supply, max 15%

// Protocol switch time of v0.3 kernel protocol
unsigned int nProtocolV03SwitchTime     = 1526519842;
unsigned int nProtocolV03TestSwitchTime = 1526519842;
// Protocol switch time of v0.4 kernel protocol
unsigned int nProtocolV04SwitchTime     = 1893456000;
unsigned int nProtocolV04TestSwitchTime = 1893456000;
// Protocol switch time of v0.5 kernel protocol
unsigned int nProtocolV05SwitchTime     = 1896134400;
unsigned int nProtocolV05TestSwitchTime = 1896134400;
// Protocol switch time of v0.6 kernel protocol
// supermajority hardfork: actual fork will happen later than switch time
const unsigned int nProtocolV06SwitchTime     = 1898553600; // Tue 12 Dec 03:40:00 UTC 2017
const unsigned int nProtocolV06TestSwitchTime = 1898553600; // Tue 17 Oct 00:00:00 UTC 2017
// Protocol switch time for 0.7 kernel protocol
const unsigned int nProtocolV07SwitchTime     = 1901228400; // Tue 12 Mar 12:00:00 UTC 2019
const unsigned int nProtocolV07TestSwitchTime = 1901228400; // Tue 06 Nov 12:00:00 UTC 2018
// Switch time for new BIPs from bitcoin 0.16.x
const uint32_t nBTC16BIPsSwitchTime           = 1903820400; // Tue 01 Oct 12:00:00 UTC 2019
const uint32_t nBTC16BIPsTestSwitchTime       = 1903820400; // Tue 09 Apr 12:00:00 UTC 2019
// Protocol switch time for v0.9 kernel protocol
const unsigned int nProtocolV09SwitchTime     = 1906498800; // Mon  8 Jun 12:00:00 UTC 2020
const unsigned int nProtocolV09TestSwitchTime = 1906498800; // Mon 17 Feb 12:00:00 UTC 2020
// Protocol switch time for v10 kernel protocol
const unsigned int nProtocolV10SwitchTime     = 1909090800; // Mon  1 Nov 12:00:00 UTC 2021
const unsigned int nProtocolV10TestSwitchTime = 1909090800; // Thu  1 Jul 12:00:00 UTC 2021
// Protocol switch time for v12 kernel protocol
const unsigned int nProtocolV12SwitchTime     = 1911769200; // Sat 18 Nov 02:58:51 UTC 2023
const unsigned int nProtocolV12TestSwitchTime = 1911769200; // Wed 14 Dec 11:23:34 UTC 2022
// Protocol switch time for v14 kernel protocol
const unsigned int nProtocolV14SwitchTime     = 1914447600; // Mon  3 Jun 12:00:00 UTC 2024
const unsigned int nProtocolV14TestSwitchTime = 1914447600; // Mon 18 Mar 00:00:00 UTC 2024
// Protocol switch time for v15 kernel protocol
const unsigned int nProtocolV15SwitchTime     = 1917039600; // Wed 12 Mar 12:00:00 UTC 2025
const unsigned int nProtocolV15TestSwitchTime = 1917039600; // Thu 12 Dec 12:00:00 UTC 2024

// Hard checkpoints of stake modifiers to ensure they are deterministic
static std::map<int, unsigned int> mapStakeModifierCheckpoints =
    boost::assign::map_list_of
    (       0, 0xfd11f4e7u)
    (    5000, 0xf74a3943u)
    (   10000, 0xac0f862bu)
    (   15000, 0x876e2889u)
    (   20000, 0x5f2c0723u)
    (   25000, 0x8265b6cbu)
    (   30000, 0x990ebc74u)
    (   35000, 0x7c3f72e3u)
    (   40000, 0xdd881acau)
    (   45000, 0xf7021c20u)
    (   50000, 0x40089ae3u)
    (   55000, 0x78a05fc6u)
    (   60000, 0x33911151u)
    (   65000, 0xba78795du)
    (   70000, 0x17fba4d6u)
    (   75000, 0xd5fd53f9u)
    (   80000, 0x33ed9cdeu)
    (   85000, 0x4313b75au)
    (   90000, 0xfbd11d6cu)
    (   95000, 0x88179ef4u)
    (  100000, 0x2f5758b2u)
    (  105000, 0xb528ba90u)
    (  110000, 0x99ced868u)
    (  115000, 0x94ab2bd8u)
    (  120000, 0x1f9611a2u)
    (  125000, 0x847d61d0u)
    (  130000, 0xfc366e28u)
    (  135000, 0x33e0433fu)
    (  140000, 0x2fc51155u)
    (  145000, 0x7331d1a8u)
    (  150000, 0xb78221feu)
    (  155000, 0x724467e8u)
    (  160000, 0x0e66e27cu)
    (  165000, 0x08b827e5u)
    (  170000, 0xed49be58u)
    (  175000, 0xe4f8db97u)
    (  180000, 0x0124d997u)
    (  185000, 0xd7f91555u)
    (  190000, 0x081f1d26u)
    (  195000, 0x364e2d4fu)
    (  200000, 0x3d7cdf21u)
    (  205000, 0x78e77361u)
    (  210000, 0x17232f99u)
    (  215000, 0xa605bf74u)
    (  220000, 0x8c80b8d4u)
    (  225000, 0x64a7de23u)
    (  230000, 0x9e456c06u)
    (  235000, 0x7de71afdu)
    (  240000, 0x94656c86u)
    (  245000, 0xb33782a7u)
    (  250000, 0x51d859dbu)
    (  255000, 0x44519cafu)
    (  260000, 0x9e61afaau)
    (  265000, 0xfd310e8du)
    (  270000, 0xb58f31f9u)
    (  275000, 0xba8a60a6u)
    (  280000, 0x10eb79c1u)
    (  285000, 0x8c4727a6u)
    (  290000, 0x7d771000u)
    (  295000, 0xf2f2b6c5u)
    (  300000, 0x7f737a59u)
    (  305000, 0xf4edc913u)
    (  310000, 0x514d70d5u)
    (  315000, 0xb4a5bb7du)
    (  320000, 0xdb078b96u)
    (  325000, 0x51144bccu)
    (  330000, 0xd15dcd26u)
    (  335000, 0x8623f655u)
    (  340000, 0x57de9eddu)
    (  345000, 0x61545b09u)
    (  350000, 0x66f13150u)
    (  355000, 0x0c499399u)
    (  360000, 0xafbeb6d4u)
    (  365000, 0xaf7db663u)
    (  370000, 0xed467597u)
    (  375000, 0x3a16000du)
    (  380000, 0xc9114f5au)
    (  385000, 0x61c088bcu)
    (  390000, 0x9c488e33u)
    (  395000, 0x811c3356u)
    (  400000, 0xe6ac39b0u)
    (  405000, 0xfbffc4c2u)
    (  410000, 0xcb71ecafu)
    (  415000, 0x9e529bdeu)
    (  420000, 0x656256d6u)
    (  425000, 0xed9e62c1u)
    (  430000, 0x732db6b1u)
    (  435000, 0xe6010c21u)
    (  440000, 0x8365ca47u)
    (  445000, 0xed73f37eu)
    (  450000, 0xdaf7197au)
    (  455000, 0xa48d9649u)
    (  460000, 0x88542ca7u)
    (  465000, 0x7ba85aa4u)
    (  470000, 0x087dfadeu)
    (  475000, 0x604a3061u)
    (  480000, 0x45dc3820u)
    (  485000, 0x66ae8b92u)
    (  490000, 0x3ad7070du)
    (  495000, 0xe71fd1b8u)
    (  500000, 0x587a38ceu)
    (  505000, 0x816d1814u)
    (  510000, 0xeaf53115u)
    (  515000, 0xe6d00b1fu)
    (  520000, 0xf12020cau)
    (  525000, 0xd464b7d5u)
    (  530000, 0x95789a5du)
    (  535000, 0x3d56eaccu)
    (  540000, 0x49712a22u)
    (  545000, 0xc5cf232du)
    (  550000, 0x44f82677u)
    (  555000, 0xf92bfb2cu)
    (  560000, 0x421f65c2u)
    (  565000, 0x6c33a361u)
    (  570000, 0x06a1ceaau)
    (  575000, 0xfe67eb20u)
    (  580000, 0x6d66559du)
    (  585000, 0x97aee545u)
    (  590000, 0x718cd8dfu)
    (  595000, 0x4c233a7cu)
    (  600000, 0x1e6e795eu)
    (  605000, 0x41105968u)
    (  610000, 0xc9066365u)
    (  615000, 0x93fe54dfu)
    (  620000, 0x5a68bcdeu)
    (  625000, 0x5ccbb242u)
    (  630000, 0x8f84eaceu)
    (  635000, 0xb6d59ffdu)
    (  640000, 0x05a97a62u)
    (  645000, 0xc9b22721u)
    (  650000, 0x83cbdce8u)
    (  655000, 0x21700da0u)
    (  660000, 0x2096b835u)
    (  665000, 0x71b3c52fu)
    (  670000, 0xb317c8fcu)
    (  675000, 0x4b066455u)
    (  680000, 0xf7739afdu)
    (  685000, 0xdd53a6f3u)
    (  690000, 0x54e6df84u)
    (  695000, 0x2539d73bu)
    (  700000, 0x13870f37u)
    (  705000, 0xa8b0ea31u)
    (  710000, 0x007e9ec6u)
    (  715000, 0xa654cd3du)
    (  720000, 0xd31921b2u)
    (  725000, 0x9bb33135u)
    (  730000, 0x9346d2a3u)
    (  735000, 0xa3cb8b54u)
    (  740000, 0x1730bddau)
    (  745000, 0x4a09b82au)
    (  750000, 0x32854245u)
    (  755000, 0x7c4ef117u)
    (  760000, 0xa91d7669u)
    (  765000, 0x8c20baacu)
    (  770000, 0x8b1b9ac7u)
    (  775000, 0x939fefc6u)
    (  780000, 0x2406f71au)
    (  785000, 0x91667aacu)
    (  790000, 0xa5778ed9u)
    (  795000, 0x9207da0fu)
    (  800000, 0x59861e7bu)
    (  805000, 0xb6faba94u)
    (  810000, 0x67fd0181u)
    (  815000, 0x5f5697c2u)
    (  820000, 0xd4c7fc2eu)
    (  825000, 0x7633d9b1u)
    (  830000, 0x6648e9ceu)
    (  835000, 0x00c918dfu)
    (  840000, 0x41fba6f8u)
    (  845000, 0xb4d266c7u)
    (  850000, 0x673bafdbu)
    (  855000, 0x2a87366cu)
    (  860000, 0x3ccd7e40u)
    (  865000, 0x7a5cc5b8u)
    (  870000, 0xa48950adu)
    (  875000, 0x323c55bau)
    (  880000, 0xa7cdcee2u)
    (  885000, 0xffefdc8eu)
    (  890000, 0xa96d497cu)
    (  895000, 0x56c3fa05u)
    (  900000, 0x43bf950eu)
    (  905000, 0x224b0929u)
    (  910000, 0x0a779d24u)
    (  915000, 0x17f0d525u)
    (  920000, 0x3a6152f9u)
    (  925000, 0xd7c256bau)
    (  930000, 0xcdf44e9du)
    (  935000, 0xe5ce1ee4u)
    (  940000, 0xd6ae4ab4u)
    (  945000, 0xc2062901u)
    (  950000, 0x24add8d1u)
    (  955000, 0xe435aeddu)
    (  960000, 0xf42d1589u)
    (  965000, 0x23a242c8u)
    (  970000, 0x0d2339dfu)
    (  975000, 0xbfb37220u)
    (  980000, 0x6cd4dba0u)
    (  985000, 0xd140e58eu)
    (  990000, 0x801af003u)
    (  995000, 0x3ec56117u)
    ( 1000000, 0x465d0a38u)
    ( 1005000, 0xb2787847u)
    ( 1010000, 0xeac572aeu)
    ( 1015000, 0x7103eccbu)
    ( 1020000, 0x276e14a5u)
    ( 1025000, 0x066894c7u)
    ( 1030000, 0x2bc36421u)
    ( 1035000, 0xb4e58e2au)
    ( 1040000, 0x503e5517u)
    ( 1045000, 0x39e467bcu)
    ( 1050000, 0x40b2581eu)
    ( 1055000, 0xe18a4abbu)
    ( 1060000, 0x79c2f6b8u)
    ( 1065000, 0xbdeefda8u)
    ( 1070000, 0x2ac0fbf8u)
    ( 1075000, 0x602c8885u)
    ( 1080000, 0xb66cd6bdu)
    ( 1085000, 0x4d20d298u)
    ( 1090000, 0x5e90ce98u)
    ( 1095000, 0x18d4237au)
    ( 1100000, 0x92b69d8au)
    ( 1105000, 0x8c247bc7u)
    ( 1110000, 0x76a880bdu)
    ( 1115000, 0xd04d090au)
    ( 1120000, 0xb6a865dau)
    ( 1125000, 0x414f016du)
    ( 1130000, 0xba8ebb49u)
    ( 1135000, 0x19ee7337u)
    ( 1140000, 0xec410e2du)
    ( 1145000, 0x9a29ff66u)
    ( 1150000, 0xd76508acu)
    ( 1155000, 0x1fe90052u)
    ( 1160000, 0x29daf3c3u)
    ( 1165000, 0x65b388eeu)
    ( 1170000, 0x61abf173u)
    ( 1175000, 0x3ebb4ae7u)
    ( 1180000, 0x263a461eu)
    ( 1185000, 0xf3967ad0u)
    ( 1190000, 0x91bad2ebu)
    ( 1195000, 0x57664a8au)
    ( 1200000, 0x68dc64c0u)
    ( 1205000, 0x26b25fa3u)
    ( 1210000, 0xef3b03adu)
    ( 1215000, 0xc652887cu)
    ( 1220000, 0x1518ca38u)
    ( 1225000, 0xa908d4c7u)
    ( 1230000, 0xf97540a0u)
    ( 1235000, 0xe991f3d8u)
    ( 1240000, 0x46ccec56u)
    ( 1245000, 0x64c54407u)
    ( 1250000, 0xc348704du)
    ( 1255000, 0xb49b88d6u)
    ( 1260000, 0xe56669c3u)
    ( 1265000, 0xbd0893a0u)
    ( 1270000, 0xcee67fc8u)
    ( 1275000, 0xaa81ed05u)
    ( 1280000, 0x63839bc5u)
    ( 1285000, 0xe9b20331u)
    ( 1290000, 0xbfb8dd8bu)
    ( 1295000, 0x70a51ccau)
    ( 1300000, 0xe85e5e75u)
    ( 1305000, 0xa6a4cedbu)
    ( 1310000, 0x1f72b29eu)
    ( 1315000, 0xda944bc9u)
    ( 1320000, 0x9c721495u)
    ( 1325000, 0xdbba59a6u)
    ( 1330000, 0xc7fb33eeu)
    ( 1335000, 0x85a6250eu)
    ( 1340000, 0x677f14bdu)
    ( 1345000, 0xc2256793u)
    ( 1350000, 0x1859a2f2u)
    ( 1355000, 0xce3e5a80u)
    ( 1360000, 0x7894f78cu)
    ( 1365000, 0x8616c256u)
    ( 1370000, 0x805fefd5u)
    ( 1375000, 0x7fbbfb33u)
    ( 1380000, 0xf14f799au)
    ( 1385000, 0xd5ee7f26u)
    ( 1390000, 0x49bcba11u)
    ( 1395000, 0x7b9100eau)
    ( 1400000, 0x5dabf8ddu)
    ( 1405000, 0x780bbd6fu)
    ( 1410000, 0xd51cdb98u)
    ( 1415000, 0x886a0d88u)
    ( 1420000, 0xb6c936a9u)
    ( 1425000, 0x4961debbu)
    ( 1430000, 0xd9d792b0u)
    ( 1435000, 0xfea1dbf1u)
    ( 1440000, 0x8b307a8au)
    ( 1445000, 0xd125289au)
    ( 1450000, 0x26dda24cu)
    ( 1455000, 0xbdec8bbeu)
    ( 1460000, 0xd2e9ef8cu)
    ( 1465000, 0xf4c33445u)
    ( 1470000, 0xdc3e22cbu)
    ( 1475000, 0x8070183au)
    ( 1480000, 0x8f301d38u)
    ( 1485000, 0xa8e33b4du)
    ( 1490000, 0xb1db0858u)
    ( 1495000, 0xd1930c85u)
    ( 1500000, 0x870b95bbu)
    ( 1505000, 0x980b1bbau)
    ( 1510000, 0x8399b8c9u)
    ( 1515000, 0xb9b1d162u)
    ( 1520000, 0x0df3978eu)
    ( 1525000, 0xbd6adf10u)
    ( 1530000, 0x8af8ecb5u)
    ( 1535000, 0x8618e87cu)
    ( 1540000, 0x06e38826u)
    ( 1545000, 0x64be802au)
    ( 1550000, 0xf109327fu)
    ( 1555000, 0x1da4c2e2u)
    ( 1560000, 0x96ee6318u)
    ( 1565000, 0xb50977feu)
    ( 1570000, 0x44fc89b9u)
    ( 1575000, 0x0de62a33u)
    ( 1580000, 0x71cc6026u)
    ( 1585000, 0x5e0de8c4u)
    ( 1590000, 0xe8b2bbcau)
    ( 1595000, 0x4a78a64bu)
    ( 1600000, 0x4d573f7bu)
    ( 1605000, 0x7ccd8839u)
    ( 1610000, 0x228c8175u)
    ( 1615000, 0x1a763869u)
    ( 1620000, 0x8b1b3968u)
    ( 1625000, 0xb0518786u)
    ( 1630000, 0x31357978u)
    ( 1635000, 0x2d1dcf43u)
    ( 1640000, 0xaff6fa5eu)
    ( 1645000, 0x6065303eu)
    ( 1650000, 0x72917e79u)
    ( 1655000, 0x4bb15216u)
    ( 1660000, 0xac22c804u)
    ( 1665000, 0xbc381c00u)
    ( 1670000, 0x5157dc73u)
    ( 1675000, 0xcf239ba1u)
    ( 1680000, 0x94070e04u)
    ( 1685000, 0xee6fbc2au)
    ( 1690000, 0x5233f6ddu)
    ( 1695000, 0x43da8d90u)
    ( 1700000, 0x2b3c38cbu)
    ( 1705000, 0xc20d17bau)
    ( 1710000, 0x67edab27u)
    ( 1715000, 0xe9cf521eu)
    ( 1720000, 0xef9b832bu)
    ( 1725000, 0xe478bcf7u)
    ( 1730000, 0xe0a8e803u)
    ( 1735000, 0x48d96f1bu)
    ( 1740000, 0xae16a659u)
    ( 1745000, 0x6efdc12cu)
    ( 1750000, 0x8b1cd1a0u)
    ( 1755000, 0xedce39f4u)
    ( 1760000, 0x62e6a5edu)
    ( 1765000, 0x25fa2b17u)
    ( 1770000, 0x94c889f0u)
    ( 1775000, 0x0b1ca518u)
    ( 1780000, 0x1b99febbu)
    ( 1785000, 0x8ebfc7b2u)
    ( 1790000, 0x09b32cb8u)
    ( 1795000, 0x28c3814du)
    ( 1800000, 0xd4103c45u)
    ( 1805000, 0x4ada5d67u)
    ( 1810000, 0x2af73365u)
    ( 1815000, 0xc4637b7du)
    ( 1820000, 0x9a2d0e37u)
    ( 1825000, 0x73f1e57au)
    ( 1830000, 0xfe366585u)
    ( 1835000, 0x709d0c87u)
    ( 1840000, 0x542c4250u)
    ( 1845000, 0xf28b4062u)
    ( 1850000, 0x3cd87156u)
    ( 1855000, 0xf72f5555u)
    ( 1860000, 0xf68128b7u)
    ( 1865000, 0x194eb69du)
    ( 1870000, 0xba5c033eu)
    ( 1875000, 0x5367e5cfu)
    ( 1880000, 0x98570bc9u)
    ( 1885000, 0x15a076d2u)
    ( 1890000, 0x005e95e3u)
    ( 1895000, 0x1e74fa2bu)
    ( 1900000, 0x91d4b7c5u)
    ( 1905000, 0x269a0ed6u)
    ( 1910000, 0xc6034609u)
    ( 1915000, 0x09593095u)
    ( 1920000, 0x2ca696c0u)
    ( 1925000, 0x55c20575u)
    ( 1930000, 0x30b1ff5eu)
    ( 1935000, 0x4b9c09a7u)
    ( 1940000, 0x38069296u)
    ( 1945000, 0x93ac596eu)
    ( 1950000, 0xcf06b601u)
    ( 1955000, 0x72036c4bu)
    ( 1960000, 0x00367835u)
    ( 1965000, 0xafe9dee2u)
    ( 1970000, 0x5a4e5a5cu)
    ( 1975000, 0x42290c2bu)
    ( 1980000, 0xb6d8dc7du)
    ( 1985000, 0x52ba1affu)
    ( 1990000, 0x3c703201u)
    ( 1995000, 0x51db5a8eu)
    ( 2000000, 0xfa658612u)
    ( 2005000, 0x29f998dfu)
    ( 2010000, 0x77e6f8c7u)
    ( 2015000, 0x891656f7u)
    ( 2020000, 0x4863381au)
    ( 2025000, 0x69c5d53du)
    ( 2030000, 0xc8011bb5u)
    ( 2035000, 0xb87f87f6u)
    ( 2040000, 0xdcc2e82bu)
    ( 2045000, 0xb5171c9du)
    ( 2050000, 0xed7e6b3cu)
    ( 2055000, 0xf7081fc0u)
    ( 2060000, 0x6c210dc7u)
    ( 2065000, 0xf0aeeca7u)
    ( 2070000, 0xc95b0fc9u)
    ( 2075000, 0xeb8a799eu)
    ( 2080000, 0xedcfb829u)
    ( 2085000, 0x2556d391u)
    ( 2090000, 0x8882047cu)
    ( 2095000, 0x9e8eb284u)
    ( 2100000, 0x3cd8fb50u)
    ( 2105000, 0x9f190d84u)
    ( 2110000, 0xc51f3cb2u)
    ( 2115000, 0xb16bc5f6u)
    ( 2120000, 0x42f7eb6cu)
    ( 2125000, 0x5c0ec508u)
    ( 2130000, 0xb20466edu)
    ( 2135000, 0x5f0ddbd9u)
    ( 2140000, 0x32a38660u)
    ( 2145000, 0xb9fbe9fau)
    ( 2150000, 0x64a78376u)
    ( 2155000, 0xa63bfc18u)
    ( 2160000, 0x8d136965u)
    ( 2165000, 0x5f0567f2u)
    ( 2170000, 0x8d3220f1u)
    ( 2175000, 0xac4f3a77u)
    ( 2180000, 0x7f72b61fu)
    ( 2185000, 0x9c6d3536u)
    ( 2190000, 0xd208e715u)
    ( 2195000, 0x9e6b3808u)
    ( 2200000, 0x98005b77u)
    ( 2205000, 0x9107543fu)
    ( 2210000, 0x45b090dau)
    ( 2215000, 0xc573c078u)
    ( 2220000, 0xbe7a2274u)
    ( 2225000, 0x012480edu)
    ( 2230000, 0xf02a6f6du)
    ( 2235000, 0xc658060bu)
    ( 2240000, 0x5f473098u)
    ( 2245000, 0x7ae863b5u)
    ( 2250000, 0xc5df4636u)
    ( 2255000, 0xfc0f9ce2u)
    ( 2260000, 0x2f8ecaa1u)
    ( 2265000, 0x130a775fu)
    ( 2270000, 0xf8033488u)
    ( 2275000, 0xf8177294u)
    ( 2280000, 0x1be11b2cu)
    ( 2285000, 0x0de92415u)
    ( 2290000, 0x6c5a6a20u)
    ( 2295000, 0x12039742u)
    ( 2300000, 0x46ef0166u)
    ( 2305000, 0x38746f1du)
    ( 2310000, 0xfc780d66u)
    ( 2315000, 0x6fe06da5u)
    ( 2320000, 0xb169c303u)
    ( 2325000, 0xce16a9f0u)
    ( 2330000, 0x674b9f77u)
    ( 2335000, 0x894455d0u)
    ( 2340000, 0x1b97405fu)
    ( 2345000, 0xff0b233du)
    ( 2350000, 0x2f5750dau)
    ( 2355000, 0xcfab92a9u)
    ( 2360000, 0xd4e779c3u)
    ( 2365000, 0x7750b8f5u)
    ( 2370000, 0xf4558d65u)
    ( 2375000, 0xc15899ceu)
    ( 2380000, 0x01a9c715u)
    ( 2385000, 0x43db5f6bu)
    ( 2390000, 0x1e87ec10u)
    ( 2395000, 0xcf9f21cau)
    ( 2400000, 0x127af797u)
    ( 2405000, 0x2916b40eu)
    ( 2410000, 0x3b75403au)
    ( 2415000, 0xc26d589du)
    ( 2420000, 0xb0193d08u)
    ( 2425000, 0xd2c5a17au)
    ( 2430000, 0x2a55da17u)
    ( 2435000, 0xf8e86425u)
    ( 2440000, 0x972b9138u)
    ( 2445000, 0xdac8a6a7u)
    ( 2450000, 0x930e076eu)
    ( 2455000, 0x8d02f4e5u)
    ( 2460000, 0x89e10cccu)
    ( 2465000, 0x828fbf34u)
    ( 2470000, 0x180c3c9fu)
    ( 2475000, 0x38967695u)
    ( 2480000, 0x46869760u)
    ( 2485000, 0x18dc3bd9u)
    ( 2490000, 0x7913d55fu)
    ( 2495000, 0xc671fe54u)
    ( 2500000, 0xe4441389u)
    ( 2505000, 0x0051c14au)
    ( 2510000, 0xc404a9c3u)
    ( 2515000, 0x44c0a5c0u)
    ( 2520000, 0x833b2116u)
    ( 2525000, 0x16878558u)
    ( 2530000, 0x4a531453u)
    ( 2535000, 0xe9c20b0fu)
    ( 2540000, 0x97ced983u)
    ( 2545000, 0x8a7db485u)
    ( 2550000, 0xeb68ea88u)
    ( 2555000, 0xb6f4f2a8u)
    ( 2560000, 0x53d26073u)
    ( 2565000, 0x71378bbcu)
    ( 2570000, 0x758a2e08u)
    ( 2575000, 0xcde8379du)
    ( 2580000, 0x2cc086b7u)
    ( 2585000, 0x43bc9552u)
    ( 2590000, 0x48076c50u)
    ( 2595000, 0xffd41830u)
    ( 2600000, 0x570bde30u)
    ( 2605000, 0x36b55b54u)
    ( 2610000, 0x433ef442u)
    ( 2615000, 0x630c201cu)
    ( 2620000, 0xfd06f74fu)
    ( 2625000, 0x8b8eb2a2u)
    ( 2630000, 0xf99a3428u)
    ( 2635000, 0x41364d8fu)
    ( 2640000, 0xad64286fu)
    ( 2645000, 0x63ceb532u)
    ( 2650000, 0x522b6badu)
    ( 2655000, 0x2aa2e47eu)
    ( 2660000, 0x170d73deu)
    ( 2665000, 0xd09aaa57u)
    ( 2670000, 0x27f99df3u)
    ( 2675000, 0xbd294950u)
    ( 2680000, 0xaec2724au)
    ( 2685000, 0x5a99c1b5u)
    ( 2690000, 0x4dcf68bfu)
    ( 2695000, 0xe3c23a8cu)
    ( 2700000, 0x448eaf13u)
    ( 2705000, 0x659853c1u)
    ( 2710000, 0x081306ccu)
    ( 2715000, 0x21aef040u)
    ( 2720000, 0x8430fbc7u)
    ( 2725000, 0x491bcf30u)
    ( 2730000, 0x73442442u)
    ( 2735000, 0xd75656d2u)
    ( 2740000, 0xae14294eu)
    ( 2745000, 0xb042ee8au)
    ( 2750000, 0x1e80f979u)
    ( 2755000, 0x2c32555bu)
    ( 2760000, 0xd6122db1u)
    ( 2765000, 0x7bfb4c93u)
    ( 2770000, 0xe703892fu)
    ( 2775000, 0x67c06835u)
    ( 2780000, 0xe19c9ab8u)
    ( 2785000, 0x12172e8au)
    ( 2790000, 0x814fb802u)
    ( 2795000, 0xe269a506u)
    ( 2800000, 0xa4354436u)
    ( 2805000, 0x1d13d2f8u)
    ( 2810000, 0x38fa8ddfu)
    ( 2815000, 0x7580d6a4u)
    ( 2820000, 0x65ba766du)
    ( 2825000, 0x621a46ddu)
    ( 2830000, 0x1fe96416u)
    ( 2835000, 0x2389233eu)
    ( 2840000, 0xdea320f1u)
    ( 2845000, 0xfa0d2c7du)
    ( 2850000, 0xd90bd361u)
    ( 2855000, 0x162182fdu)
    ( 2860000, 0x10e8e8f8u)
    ( 2865000, 0x9f755951u)
    ( 2870000, 0xde806474u)
    ( 2875000, 0x661bfa70u)
    ( 2880000, 0x01f30dfcu)
    ( 2885000, 0xaf5c0da3u)
    ( 2890000, 0xf0c31da5u)
    ( 2895000, 0x704d267fu)
    ( 2900000, 0xe5594854u)
    ( 2905000, 0x8855b888u)
    ( 2910000, 0xf589a70bu)
    ( 2915000, 0x36276b2cu)
    ( 2920000, 0x3af8fe00u)
    ( 2925000, 0x555efde6u)
    ( 2930000, 0x77aa10fcu)
    ( 2935000, 0x2cc88976u)
    ( 2940000, 0x5e179b26u)
    ( 2945000, 0x57cb26b8u)
    ( 2950000, 0x891da0a7u)
    ( 2955000, 0x00bc7d43u)
    ( 2960000, 0xd69cf35au)
    ( 2965000, 0x682d6b59u)
    ( 2970000, 0xcc6d4615u)
    ( 2975000, 0x8ca39bdeu)
    ( 2980000, 0x9773627bu)
    ( 2985000, 0xb85d4b84u)
    ( 2990000, 0xfbd78a95u)
    ( 2995000, 0xa39686ccu)
    ( 3000000, 0x7c2abfddu)
    ( 3005000, 0x9f110197u)
    ( 3010000, 0x5ca20535u)
    ( 3015000, 0x4404ac4cu)
    ( 3020000, 0x609c2e61u)
    ( 3025000, 0xdbdc8b2du)
    ( 3030000, 0x09f61a67u)
    ( 3035000, 0x8e641407u)
    ( 3040000, 0xbd0cc91fu)
    ( 3045000, 0xcd2608f9u)
    ( 3050000, 0xd3e71cf4u)
    ( 3055000, 0xf1ace0fdu)
    ( 3060000, 0xa6b1409du)
    ( 3065000, 0x022ae8beu)
    ( 3070000, 0x48afab16u)
    ( 3075000, 0x16e706d2u)
    ( 3080000, 0x70fa5d1fu)
    ( 3085000, 0x51ec05aau)
    ( 3090000, 0x29866e5bu)
    ( 3095000, 0x6eb831cau)
    ( 3100000, 0x0860a3ffu)
    ( 3105000, 0x94c6966au)
    ( 3110000, 0x9ff8aa94u)
    ( 3115000, 0x3169961cu)
    ( 3120000, 0x5885cee1u)
    ( 3125000, 0x43ca330cu)
    ( 3130000, 0x895f2c74u)
    ( 3135000, 0x939d0114u)
    ( 3140000, 0xdcc7d1d2u)
    ( 3145000, 0x424fce4eu)
    ( 3150000, 0xe1e9b7cfu)
    ( 3155000, 0xe1b8f61bu)
    ( 3160000, 0x773034d1u)
    ( 3165000, 0xab184c8fu)
    ( 3170000, 0x2b3bed68u)
    ( 3175000, 0xafd2779fu)
    ( 3180000, 0x18b73cb9u)
    ( 3185000, 0x3713068cu)
    ( 3190000, 0x1d7547bdu)
    ( 3195000, 0x7e13f7bdu)
    ( 3200000, 0xdfb5c725u)
    ( 3205000, 0xc744359eu)
    ( 3210000, 0x7c4bad10u)
    ( 3215000, 0x8cdc41cdu)
    ( 3220000, 0x7a50afebu)
    ( 3225000, 0xf59faf83u)
    ( 3230000, 0x311ade20u)
    ( 3235000, 0x8093946cu)
    ( 3240000, 0x0d0cf6efu)
    ( 3245000, 0x1bb0a0c1u)
    ( 3250000, 0x74b510a1u)
    ( 3255000, 0xc78a78f0u)
    ( 3260000, 0x445d6305u)
    ( 3265000, 0x75114a1au)
    ( 3270000, 0xe8d6ebd4u)
    ( 3275000, 0xf58c5b2eu)
    ( 3280000, 0xd68a35e4u)
    ( 3285000, 0xcb335c82u)
    ( 3290000, 0xa95bf0a3u)
    ( 3295000, 0xb3cfb913u)
    ( 3300000, 0xdfd70b4cu)
    ( 3305000, 0x400fad50u)
    ( 3310000, 0x0160e8f7u)
    ( 3315000, 0x11fa0cd7u)
    ( 3320000, 0x1a7a07beu)
    ( 3325000, 0x5f8c87d0u)
    ( 3330000, 0x1451210eu)
    ( 3335000, 0xa0941237u)
    ( 3340000, 0x6f734ebdu)
    ( 3345000, 0x3d371e2cu)
    ( 3350000, 0x7f9d862bu)
    ( 3355000, 0xa79a4e80u)
    ( 3360000, 0x00e2e890u)
    ( 3365000, 0x8808a9ccu)
    ( 3370000, 0x2852ef6fu)
    ( 3375000, 0x2bf80580u)
    ( 3380000, 0xf37137b6u)
    ( 3385000, 0xd83d8c9fu)
    ( 3390000, 0x23a11e87u)
    ( 3395000, 0x28efc586u)
    ( 3400000, 0x86cd798du)
    ( 3405000, 0x9d27fc06u)
    ( 3410000, 0x7007b86au)
    ( 3415000, 0x4e3e936cu)
    ( 3420000, 0xf33fa879u)
    ( 3425000, 0x56a1e261u)
    ( 3430000, 0x95298df0u)
    ( 3435000, 0x539977d9u)
    ( 3440000, 0x9ad25836u)
    ( 3445000, 0x1be1869du)
    ( 3450000, 0x4b024649u)
    ( 3455000, 0x97f1861cu)
    ( 3460000, 0x138c7234u)
    ( 3465000, 0xcb91194cu)
    ( 3470000, 0x7f90b9f5u)
    ( 3475000, 0x6b7e8ef3u)
    ( 3480000, 0xfaca3da6u)
    ( 3485000, 0x51f6423au)
    ( 3490000, 0xeea41eedu)
    ( 3495000, 0x80643796u)
    ( 3500000, 0x089e5abcu)
    ( 3505000, 0xfdd7e19eu)
    ( 3510000, 0xc3115669u)
    ( 3515000, 0xe5760682u)
    ( 3520000, 0xeefd3163u)
    ( 3525000, 0x8a7e97aeu)
    ( 3530000, 0xe0984a02u)
    ( 3535000, 0x909192d2u)
    ( 3540000, 0x2daa64aeu)
    ( 3545000, 0x14a1ca7cu)
    ( 3550000, 0xeb78d70eu)
    ( 3555000, 0x7c28a957u)
    ( 3560000, 0x3beb1a17u)
    ( 3565000, 0x267e7cdau)
    ( 3570000, 0xb3cc2538u)
    ( 3575000, 0x6eaa6294u)
    ( 3580000, 0x27157e26u)
    ( 3585000, 0x6e15e86cu)
    ( 3590000, 0xb22f2da9u)
    ( 3595000, 0x990d485eu)
    ( 3600000, 0x346544f8u)
    ( 3605000, 0xde5e6b8cu)
    ( 3610000, 0x6a23b66cu)
    ( 3615000, 0x6a2baaecu)
    ( 3620000, 0x526cf048u)
    ( 3625000, 0x05dc7657u)
    ( 3630000, 0xf107d35eu)
    ( 3635000, 0x0c465ad6u)
    ( 3640000, 0xca8d7ad5u)
    ( 3645000, 0x96045851u)
    ( 3650000, 0xe541eec9u)
    ( 3655000, 0x69b7d2cdu)
    ( 3660000, 0x45d2f385u)
    ( 3665000, 0x6eb82b59u)
    ( 3670000, 0xe44ecf8cu)
    ( 3675000, 0x5d917b99u)
    ( 3680000, 0xb6456b16u)
    ( 3685000, 0xf7991508u)
    ( 3690000, 0x0fcc8ae6u)
    ( 3695000, 0x9c33c993u)
    ( 3700000, 0x3cad2234u)
    ( 3705000, 0xe4f4d9b7u)
    ( 3710000, 0xbd6cae08u)
    ( 3715000, 0xa6d63070u)
    ( 3720000, 0xce8d59e5u)
    ( 3725000, 0x58056d11u)
    ( 3730000, 0xd4d65f4fu)
    ( 3735000, 0xf5706d43u)
    ( 3740000, 0x1a9f11e0u)
    ( 3745000, 0x05e53a9bu)
    ( 3750000, 0x8ddc5571u)
    ( 3755000, 0xa5238e59u)
    ( 3760000, 0x24fd6cbbu)
    ( 3765000, 0x0c4ebf3au)
    ( 3770000, 0x9f301be6u)
    ( 3775000, 0x7be8ab0fu)
    ( 3780000, 0x26509897u)
    ( 3785000, 0x52ae7a28u)
    ( 3790000, 0xc5e1a2a2u)
    ( 3795000, 0xe0a315bcu)
    ( 3800000, 0x0f3092dbu)
    ( 3805000, 0x94b16616u)
    ( 3810000, 0xbcd954acu)
    ( 3815000, 0x52781edfu)
    ( 3820000, 0xcb73f6e0u)
    ( 3825000, 0x0f6b5f11u)
    ( 3830000, 0x586dbc5eu)
    ( 3835000, 0x45f9e626u)
    ( 3840000, 0x084b952cu)
    ( 3845000, 0xb2a01de6u)
    ( 3850000, 0x6d789e63u)
    ( 3855000, 0x671a5989u)
    ( 3860000, 0xd2226208u)
    ( 3865000, 0xe54212f0u)
    ( 3870000, 0x813816c9u)
    ( 3875000, 0xc81be0e8u)
    ( 3880000, 0x97c26b8au)
    ( 3885000, 0x0f2a9b11u)
    ( 3890000, 0xe564b4e1u)
    ( 3895000, 0x30303050u)
    ( 3900000, 0xab6bc62fu)
    ( 3905000, 0x749537efu)
    ( 3910000, 0x034c55ccu)
    ( 3915000, 0x0cc3dda2u)
    ( 3920000, 0x282b4abfu)
    ( 3925000, 0xee98c365u)
    ( 3930000, 0xd35d175bu)
    ( 3935000, 0xf44630bcu)
    ( 3940000, 0x998715d2u)
    ( 3945000, 0x8b18dee4u)
    ( 3950000, 0x24819154u)
    ( 3955000, 0x82b1e09cu)
    ( 3960000, 0x1ecce33cu)
    ( 3965000, 0x7bc08627u)
    ( 3970000, 0xc180b759u)
    ( 3975000, 0x5ece844au)
    ( 3980000, 0x2961c5e3u)
    ( 3985000, 0x05f7ef0eu)
    ( 3990000, 0xe23e909fu)
    ( 3995000, 0xe3aa4e20u)
    ( 4000000, 0xfcf2ddd5u)
    ( 4005000, 0xc5545807u)
    ( 4010000, 0x22d80ec4u)
    ( 4015000, 0x435a96cdu)
    ( 4020000, 0x7c31fa4du)
    ( 4025000, 0x87f2f653u)
    ( 4030000, 0xe8860fd1u)
    ( 4035000, 0x2c68d261u)
    ( 4040000, 0xf72ed1ebu)
    ( 4045000, 0xee1bb998u)
    ( 4050000, 0x8f6551feu)
    ( 4055000, 0xc66b5c8du)
    ( 4060000, 0x613f734cu)
    ( 4065000, 0xc2a227bbu)
    ( 4070000, 0x315b5a2bu)
    ( 4075000, 0xb16de294u)
    ( 4080000, 0xb650fd42u)
    ( 4085000, 0x9f3fd9e9u)
    ( 4090000, 0x9902c971u)
    ( 4095000, 0xd3182676u)
    ( 4100000, 0xbbaca246u)
    ( 4105000, 0x6dc75c8au)
    ( 4110000, 0xa79d8ee4u)
    ( 4115000, 0x5952a6b7u)
    ( 4120000, 0x0e164f48u)
    ( 4125000, 0x226b4321u)
    ( 4130000, 0xf929b3a8u)
    ( 4135000, 0x1aa1caa7u)
    ( 4140000, 0x5a902e72u)
    ( 4145000, 0x93cfa621u)
    ( 4150000, 0xd884ac99u)
    ( 4155000, 0x373bcf16u)
    ( 4160000, 0x42f0dba9u)
    ( 4165000, 0x5567c408u)
    ( 4170000, 0x7f1c1f9cu)
    ( 4175000, 0x06c85f8eu)
    ( 4180000, 0x618705eeu)
    ( 4185000, 0x9f0e67e8u)
    ( 4190000, 0x7472a8e5u)
    ( 4195000, 0x9dac4793u)
    ( 4200000, 0x42845293u)
    ( 4205000, 0xa6f0588eu)
    ( 4210000, 0x6576e0f2u)
    ( 4215000, 0x21a752adu)
    ( 4220000, 0x76dc96ddu)
    ( 4225000, 0x8de7c937u)
    ( 4230000, 0x58607a51u)
    ( 4235000, 0xd413f5d0u)
    ( 4240000, 0xcef9acf4u)
    ( 4245000, 0xf7bd6fafu)
    ( 4250000, 0x12129dcau)
    ( 4255000, 0x9ca37630u)
    ( 4260000, 0xf4988148u)
    ( 4265000, 0xd531877eu)
    ( 4270000, 0x9c74c6d2u)
    ( 4275000, 0x0013f645u)
    ( 4280000, 0x67719ebbu)
    ( 4285000, 0xbe689585u)
    ( 4290000, 0xa0dffb1bu)
    ( 4295000, 0x7e67fdb5u)
    ( 4300000, 0xbad970e4u)
    ( 4305000, 0x185d6138u)
    ( 4310000, 0x8e8d4061u)
    ( 4315000, 0x855de024u)
    ( 4320000, 0x1ddca097u)
    ( 4325000, 0xe1b6c7bdu)
    ( 4330000, 0x7741ddd5u)
    ( 4335000, 0xf540e5ceu)
    ( 4340000, 0x75789fd7u)
    ( 4345000, 0x50f63c76u)
    ( 4350000, 0xcea41b98u)
    ( 4355000, 0xc8ce6860u)
    ( 4360000, 0x85c9e257u)
    ( 4365000, 0xde5005a8u)
    ( 4370000, 0x6f001c76u)
    ( 4375000, 0xc150d659u)
    ( 4380000, 0x017609abu)
    ( 4385000, 0xad6632bdu)
    ( 4390000, 0xdca3ca64u)
    ( 4395000, 0x6698d6b2u)
    ( 4400000, 0xfab75f76u)
    ( 4405000, 0x701991cdu)
    ( 4410000, 0x93fff62du)
    ( 4415000, 0xf58bd803u)
    ( 4420000, 0x1df0ad3au)
    ( 4425000, 0x9d9e6ccau)
    ( 4430000, 0xb214ddfbu)
    ( 4435000, 0x5d5e1164u)
    ( 4440000, 0x5d195cc1u)
    ( 4445000, 0xed8fd2fdu)
    ( 4450000, 0x7b323777u)
    ( 4455000, 0xc40e1ac9u)
    ( 4460000, 0xe4e1f25bu)
    ( 4465000, 0x21956c16u)
    ( 4470000, 0x080c3f91u)
    ( 4475000, 0x737a8901u)
    ( 4480000, 0xfb792797u)
    ( 4485000, 0x8ea6f2a7u)
    ( 4490000, 0x3d2fc7b7u)
    ( 4495000, 0xd8b35cc7u)
    ( 4500000, 0x8649a042u)
    ( 4505000, 0x241237b9u)
    ( 4510000, 0xfba2fd0fu)
    ( 4515000, 0x23d87acau)
    ( 4520000, 0x8cbd5d7bu)
    ( 4525000, 0xe7f8b855u)
    ( 4530000, 0xf4f35ffeu)
    ( 4535000, 0x372be10au)
    ( 4540000, 0x6edff4d0u)
    ( 4545000, 0xa8c8b143u)
    ( 4550000, 0x23b594ebu)
    ( 4555000, 0x2e7d1764u)
    ( 4560000, 0x2ab88f69u)
    ( 4565000, 0x67da05c7u)
    ( 4570000, 0x7786410au)
    ( 4575000, 0x6b08269du)
    ( 4580000, 0xafe2b329u)
    ( 4585000, 0x672a6ce1u)
    ( 4590000, 0x81e2ca43u)
    ( 4595000, 0xce169810u)
    ( 4600000, 0x79238cebu)
    ( 4605000, 0x3308b1f1u)
    ( 4610000, 0x994b3f8eu)
    ( 4615000, 0xf1f7ff60u)
    ( 4620000, 0x0735f108u)
    ( 4625000, 0x1c179b3fu)
    ( 4630000, 0x8c8c048cu)
    ( 4635000, 0xe13b3176u)
    ( 4640000, 0x44d6953fu)
    ( 4645000, 0xa092218eu)
    ( 4650000, 0xfd24850fu)
    ( 4655000, 0xe17e0ee4u)
    ( 4660000, 0xa6ce01b7u)
    ( 4665000, 0xe59b1f32u)
    ( 4670000, 0xdbc54ffbu)
    ( 4675000, 0x35731d2au)
    ( 4680000, 0x0d1c4ebeu)
    ( 4685000, 0x0f8800c4u)
    ( 4690000, 0x6c9ac23du)
    ( 4695000, 0x493c9004u)
    ( 4700000, 0x801e330du)
    ( 4705000, 0xbf6bf75au)
    ( 4710000, 0xf8376049u)
    ( 4715000, 0xae137bfbu)
    ( 4720000, 0xc6b9ef40u)
    ( 4725000, 0x0fd619f8u)
    ( 4730000, 0xdd5640e2u)
    ( 4735000, 0x34c4536du)
    ( 4740000, 0x461879d7u)
    ( 4745000, 0x44ca1b16u)
    ( 4750000, 0xfbb9ebbfu)
    ( 4755000, 0xe1450660u)
    ( 4760000, 0x90cf86cfu)
    ( 4765000, 0xf1ca2155u)
    ( 4770000, 0x2f3eb013u)
    ( 4775000, 0x81ecddfau)
    ( 4780000, 0xbb654472u)
    ( 4785000, 0xb06cb725u)
    ( 4790000, 0x465e69aeu)
    ( 4795000, 0xc185c9d5u)
    ( 4800000, 0x59c2256cu)
    ( 4805000, 0x25603033u)
    ( 4810000, 0xd0739532u)
    ( 4815000, 0x9c18e8bau)
    ( 4820000, 0x0e8e9569u)
    ( 4825000, 0x1c828382u)
    ( 4830000, 0x7047d61cu)
    ( 4835000, 0xcde05a52u)
    ( 4840000, 0xfb14e3a8u)
    ( 4845000, 0x9620dbf3u)
    ( 4850000, 0xa8ed8e8bu)
    ( 4855000, 0x74508426u)
    ( 4860000, 0x37923896u)
    ( 4865000, 0xf9b94c53u)
    ( 4870000, 0x4e27cb92u)
    ( 4875000, 0xaa0867d4u)
    ( 4880000, 0x7013d168u)
    ( 4885000, 0x35a3c4dfu)
    ( 4890000, 0x2010204cu)
    ( 4895000, 0x76c3b842u)
    ( 4900000, 0xcef0a43bu)
    ( 4905000, 0x19568276u)
    ( 4910000, 0x21088f8cu)
    ( 4915000, 0x703a9b67u)
    ( 4920000, 0x0eb4a93du)
    ( 4925000, 0x460d6c71u)
    ( 4930000, 0x79f8a022u)
    ( 4935000, 0x09e0b69du)
    ( 4940000, 0xcd4d1725u)
    ( 4945000, 0x231347acu)
    ( 4950000, 0x83863511u)
    ( 4955000, 0x84a63b61u)
    ( 4960000, 0xd4370b22u)
    ( 4965000, 0x1e683c41u)
    ( 4970000, 0xe84c7c3bu)
    ( 4975000, 0x3103a4a9u)
    ( 4980000, 0x54802ecau)
    ( 4985000, 0xe2cf0517u)
    ( 4990000, 0xd150abe5u)
    ( 4995000, 0x61663351u)
    ( 5000000, 0x5c9c5180u)
    ( 5005000, 0x857d6d9bu)
    ( 5010000, 0xc5e00a2du)
    ( 5015000, 0x21dde7c7u)
    ( 5020000, 0x2ae6825du)
    ( 5025000, 0xb719e694u)
    ( 5030000, 0xac9a4173u)
    ( 5035000, 0x4a39dcddu)
    ( 5040000, 0x591a6170u)
    ( 5045000, 0x0f9fb13eu)
    ( 5050000, 0x71cc582fu)
    ( 5055000, 0x64a93debu)
    ( 5060000, 0x9aaaa706u)
    ( 5065000, 0xac2b93d9u)
    ( 5070000, 0xbff0ccadu)
    ( 5075000, 0xf003dc46u)
    ( 5080000, 0xd427be4fu)
    ( 5085000, 0x1ef059deu)
    ( 5090000, 0xc654c87cu)
    ( 5095000, 0xd3368868u)
    ( 5100000, 0x61e67930u)
    ( 5105000, 0x0f4270b2u)
    ( 5110000, 0xa8c5214cu)
    ( 5115000, 0x431e371eu)
    ( 5120000, 0xfe85d340u)
    ( 5125000, 0x7344ca57u)
    ( 5130000, 0x2ff36e94u)
    ( 5135000, 0xb99cf069u)
    ( 5140000, 0xee0b592eu)
    ( 5145000, 0xd04c70b7u)
    ( 5150000, 0x16c6d740u)
    ( 5155000, 0x28a21cacu)
    ( 5160000, 0xbf0e764eu)
    ( 5165000, 0x370aa641u)
    ( 5170000, 0xee4c57e5u)
    ( 5175000, 0x77ea32ecu)
    ( 5180000, 0x631374bdu)
    ( 5185000, 0x36529463u)
    ( 5190000, 0x572a853eu)
    ( 5195000, 0xd11242c1u)
    ( 5200000, 0xc2d38f52u)
    ( 5205000, 0x3454f845u)
    ( 5210000, 0xfddc4fefu)
    ( 5215000, 0xef1340a9u)
    ( 5220000, 0xc90f1d2fu)
    ( 5225000, 0x1c7174e4u)
    ( 5230000, 0xc4836e87u)
    ( 5235000, 0xd757f2a8u)
    ( 5240000, 0x88c9705fu)
    ( 5245000, 0x9a531079u)
    ( 5250000, 0xce4d7a5eu)
    ( 5255000, 0x53808bc6u)
    ( 5260000, 0x082ba8cbu)
    ( 5261473, 0x21aafe36u)
    ;

static std::map<int, unsigned int> mapStakeModifierTestnetCheckpoints =
    boost::assign::map_list_of
        ( 0,	0x0e00670b )
    ;

inline double wfaV2(double x)
{
    return (1 / (1 + exp_n( (x-0.045)/0.0075 ))) + 1;
}

inline double wfbV2(double x)
{
    return (1 / (1 + exp_n( (x-0.08)/0.08 ))) + 1;
}

inline double wfcV2(double x)
{
    return (1 / (1 + exp_n( (x-0.25)/0.125 )));
}

// Magi-specific functions
static bool fDebugMagiPoS = false;

inline unsigned int GetStakeMinAge(unsigned int nTime0) { return ( (nTime0 > 1503248400) ? (60 * 60 * 8) : (60 * 60 * 2) ); }

// Get time weight
int64_t GetMagiWeightV2(int64_t nValueIn, int64_t nIntervalBeginning, int64_t nIntervalEnd)
{
    double nWeight = 0;
    int64_t nnMoneySupply = MAX_MONEY_STAKE_REF_V2;

    if (nValueIn >= MAX_MONEY_STAKE_REF_V2) return 0;
    
    double rStakeDays = (double)(max((int64_t)0, nIntervalEnd - nIntervalBeginning - GetStakeMinAge(nIntervalEnd))) / (24. * 60. * 60.);
    double rMro = (double)(nValueIn*6)/(double)nnMoneySupply, rEpf = exp_n(1/wfaV2(rMro)/wfbV2(rMro)/wfcV2(rMro));

    if (rMro/6 >= MAX_MAGI_BALANCE_in_STAKE) return 0;

    nWeight = 42.2474 * ( pow(rEpf, -0.55 * (rStakeDays+2.) / 0.4719) - pow(rEpf, -0.6 * (rStakeDays+2.) / 0.4719) ) * rStakeDays;

    if (fDebugMagiPoS) LogPrintf("@GetMagiWeightV2 = %" PRId64 "\n", max((int64_t)0, min((int64_t)(nWeight * 24 * 60 * 60), (int64_t)nStakeMaxAge)));

    return max((int64_t)0, min((int64_t)(nWeight * 24 * 60 * 60), (int64_t)nStakeMaxAge));
}

// Whether the given coinstake is subject to new v0.3 protocol
bool IsProtocolV03(unsigned int nTimeCoinStake)
{
    return (nTimeCoinStake >= (Params().NetworkIDString() != CBaseChainParams::MAIN ? nProtocolV03TestSwitchTime : nProtocolV03SwitchTime));
}

// Whether the given block is subject to new v0.4 protocol
bool IsProtocolV04(unsigned int nTimeBlock)
{
    return (nTimeBlock >= (Params().NetworkIDString() != CBaseChainParams::MAIN ? nProtocolV04TestSwitchTime : nProtocolV04SwitchTime));
}

// Whether the given transaction is subject to new v0.5 protocol
bool IsProtocolV05(unsigned int nTimeTx)
{
    return (nTimeTx >= (Params().NetworkIDString() != CBaseChainParams::MAIN ? nProtocolV05TestSwitchTime : nProtocolV05SwitchTime));
}

// Whether a given block is subject to new v0.6 protocol
// Test against previous block index! (always available)
bool IsProtocolV06(const CBlockIndex* pindexPrev)
{
  if (Params().NetworkIDString() == CBaseChainParams::REGTEST)
      return true;

  if (pindexPrev->nTime < (Params().NetworkIDString() != CBaseChainParams::MAIN ? nProtocolV06TestSwitchTime : nProtocolV06SwitchTime))
    return false;

  // if 900 of the last 1,000 blocks are version 2 or greater (90/100 if testnet):
  // Soft-forking PoS can be dangerous if the super majority is too low
  // The stake majority will decrease after the fork
  // since only coindays of updated nodes will get destroyed.
  if ((Params().NetworkIDString() == CBaseChainParams::MAIN && pindexPrev->nHeight > 339678) ||
      (Params().NetworkIDString() != CBaseChainParams::MAIN && pindexPrev->nHeight > 301251))
    return true;

  return false;
}

// Whether a given transaction is subject to new v0.7 protocol
bool IsProtocolV07(unsigned int nTimeTx)
{
    bool fTestNet = Params().NetworkIDString() != CBaseChainParams::MAIN;
    return (nTimeTx >= (fTestNet? nProtocolV07TestSwitchTime : nProtocolV07SwitchTime));
}

bool IsBTC16BIPsEnabled(uint32_t nTimeTx)
{
    bool fTestNet = Params().NetworkIDString() != CBaseChainParams::MAIN;
    return (nTimeTx >= (fTestNet? nBTC16BIPsTestSwitchTime : nBTC16BIPsSwitchTime));
}

// Whether a given timestamp is subject to new v0.9 protocol
bool IsProtocolV09(unsigned int nTime)
{
  return (nTime >= (Params().NetworkIDString() != CBaseChainParams::MAIN ? nProtocolV09TestSwitchTime : nProtocolV09SwitchTime));
}

// Whether a given timestamp is subject to new v10 protocol
bool IsProtocolV10(unsigned int nTime)
{
  return (nTime >= (Params().NetworkIDString() != CBaseChainParams::MAIN ? nProtocolV10TestSwitchTime : nProtocolV10SwitchTime));
}

// Whether a given block is subject to new v12 protocol
bool IsProtocolV12(const CBlockIndex* pindexPrev)
{
  if (Params().NetworkIDString() == CBaseChainParams::REGTEST)
      return true;

  return (pindexPrev->nTime >= (Params().NetworkIDString() != CBaseChainParams::MAIN ? nProtocolV12TestSwitchTime : nProtocolV12SwitchTime));
}

// Whether a given block is subject to new v14 protocol
bool IsProtocolV14(const CBlockIndex* pindexPrev)
{
  if (Params().NetworkIDString() == CBaseChainParams::REGTEST)
      return true;

  if (pindexPrev->nTime < (Params().NetworkIDString() != CBaseChainParams::MAIN ? nProtocolV14TestSwitchTime : nProtocolV14SwitchTime))
      return false;

  if ((Params().NetworkIDString() == CBaseChainParams::MAIN && pindexPrev->nHeight > 770395) ||
      (Params().NetworkIDString() != CBaseChainParams::MAIN && pindexPrev->nHeight > 573706))
    return true;

  return false;
}

// Whether a given block is subject to new v15 protocol
bool IsProtocolV15(const CBlockIndex* pindexPrev)
{
  if (Params().NetworkIDString() == CBaseChainParams::REGTEST)
      return true;

  if (pindexPrev->nTime < (Params().NetworkIDString() != CBaseChainParams::MAIN ? nProtocolV15TestSwitchTime : nProtocolV15SwitchTime))
      return false;

  if ((Params().NetworkIDString() == CBaseChainParams::MAIN && pindexPrev->nHeight > 801330) ||
      (Params().NetworkIDString() != CBaseChainParams::MAIN && pindexPrev->nHeight > 612775))
    return true;

  return false;
}

// Get the last stake modifier and its generation time from a given block
static bool GetLastStakeModifier(const CBlockIndex* pindex, uint64_t& nStakeModifier, int64_t& nModifierTime)
{
    if (!pindex)
        return error("GetLastStakeModifier: null pindex");
    while (pindex && pindex->pprev && !pindex->GeneratedStakeModifier())
        pindex = pindex->pprev;
    if (!pindex->GeneratedStakeModifier())
        return error("GetLastStakeModifier: no generation at genesis block");
    nStakeModifier = pindex->nStakeModifier;
    nModifierTime = pindex->GetBlockTime();
    return true;
}

// Get selection interval section (in seconds)
static int64_t GetStakeModifierSelectionIntervalSection(int nSection)
{
    assert (nSection >= 0 && nSection < 64);
    return (Params().GetConsensus().nModifierInterval * 63 / (63 + ((63 - nSection) * (MODIFIER_INTERVAL_RATIO - 1))));
}

// Get stake modifier selection interval (in seconds)
static int64_t GetStakeModifierSelectionInterval()
{
    int64_t nSelectionInterval = 0;
    for (int nSection=0; nSection<64; nSection++)
        nSelectionInterval += GetStakeModifierSelectionIntervalSection(nSection);
    return nSelectionInterval;
}

// select a block from the candidate blocks in vSortedByTimestamp, excluding
// already selected blocks in vSelectedBlocks, and with timestamp up to
// nSelectionIntervalStop.
static bool SelectBlockFromCandidates(
    vector<pair<int64_t, uint256> >& vSortedByTimestamp,
    map<uint256, const CBlockIndex*>& mapSelectedBlocks,
    int64_t nSelectionIntervalStop, uint64_t nStakeModifierPrev,
    const CBlockIndex** pindexSelected,
    Chainstate& chainstate)
{
    LOCK(cs_main);
    bool fSelected = false;
    arith_uint256 hashBest = 0;
    *pindexSelected = (const CBlockIndex*) 0;
    for (const auto& item : vSortedByTimestamp)
    {
/*
        if (!chainstate.BlockIndex().count(item.second))
            return error("SelectBlockFromCandidates: failed to find block index for candidate block %s", item.second.ToString());
        const CBlockIndex* pindex = pindexSelected.BlockIndex()[item.second];
*/
        const CBlockIndex* pindex = chainstate.m_blockman.LookupBlockIndex(item.second);
        if (!pindex)
            return error("SelectBlockFromCandidates: failed to find block index for candidate block %s", item.second.ToString());

        if (fSelected && pindex->GetBlockTime() > nSelectionIntervalStop)
            break;
        if (mapSelectedBlocks.count(pindex->GetBlockHash()) > 0)
            continue;
        // compute the selection hash by hashing its proof-hash and the
        // previous proof-of-stake modifier
        uint256 hashProof = pindex->IsProofOfStake()? pindex->hashProofOfStake : pindex->GetBlockHash();
        CDataStream ss(SER_GETHASH, 0);
        ss << hashProof << nStakeModifierPrev;
        arith_uint256 hashSelection = UintToArith256(Hash(ss));
        // the selection hash is divided by 2**32 so that proof-of-stake block
        // is always favored over proof-of-work block. this is to preserve
        // the energy efficiency property
        if (pindex->IsProofOfStake())
            hashSelection >>= 32;
        if (fSelected && hashSelection < hashBest)
        {
            hashBest = hashSelection;
            *pindexSelected = (const CBlockIndex*) pindex;
        }
        else if (!fSelected)
        {
            fSelected = true;
            hashBest = hashSelection;
            *pindexSelected = (const CBlockIndex*) pindex;
        }
    }
    if (gArgs.GetBoolArg("-debug", false) && gArgs.GetBoolArg("-printstakemodifier", false))
        LogPrintf("SelectBlockFromCandidates: selection hash=%s\n", hashBest.ToString());
    return fSelected;
}

// Stake Modifier (hash modifier of proof-of-stake):
// The purpose of stake modifier is to prevent a txout (coin) owner from
// computing future proof-of-stake generated by this txout at the time
// of transaction confirmation. To meet kernel protocol, the txout
// must hash with a future stake modifier to generate the proof.
// Stake modifier consists of bits each of which is contributed from a
// selected block of a given block group in the past.
// The selection of a block is based on a hash of the block's proof-hash and
// the previous stake modifier.
// Stake modifier is recomputed at a fixed time interval instead of every
// block. This is to make it difficult for an attacker to gain control of
// additional bits in the stake modifier, even after generating a chain of
// blocks.
bool ComputeNextStakeModifier(const CBlockIndex* pindexCurrent, uint64_t &nStakeModifier, bool& fGeneratedStakeModifier, Chainstate& chainstate)
{
    const Consensus::Params& params = Params().GetConsensus();
    const CBlockIndex* pindexPrev = pindexCurrent->pprev;
    nStakeModifier = 0;
    fGeneratedStakeModifier = false;
    if (!pindexPrev)
    {
        fGeneratedStakeModifier = true;
        return true;  // genesis block's modifier is 0
    }
    // First find current stake modifier and its generation block time
    // if it's not old enough, return the same stake modifier
    int64_t nModifierTime = 0;
    if (!GetLastStakeModifier(pindexPrev, nStakeModifier, nModifierTime))
        return error("ComputeNextStakeModifier: unable to get last modifier");
    if (gArgs.GetBoolArg("-debug", false))
        LogPrintf("ComputeNextStakeModifier: prev modifier=0x%016x time=%s epoch=%u\n", nStakeModifier, FormatISO8601DateTime(nModifierTime), (unsigned int)nModifierTime);
    if (nModifierTime / params.nModifierInterval >= pindexPrev->GetBlockTime() / params.nModifierInterval)
    {
        if (gArgs.GetBoolArg("-debug", false))
            LogPrintf("ComputeNextStakeModifier: no new interval keep current modifier: pindexPrev nHeight=%d nTime=%u\n", pindexPrev->nHeight, (unsigned int)pindexPrev->GetBlockTime());
        return true;
    }
    if (nModifierTime / params.nModifierInterval >= pindexCurrent->GetBlockTime() / params.nModifierInterval)
    {
        // v0.4+ requires current block timestamp also be in a different modifier interval
        if (IsProtocolV04(pindexCurrent->nTime))
        {
            if (gArgs.GetBoolArg("-debug", false))
                LogPrintf("ComputeNextStakeModifier: (v0.4+) no new interval keep current modifier: pindexCurrent nHeight=%d nTime=%u\n", pindexCurrent->nHeight, (unsigned int)pindexCurrent->GetBlockTime());
            return true;
        }
        else
        {
            if (gArgs.GetBoolArg("-debug", false))
                LogPrintf("ComputeNextStakeModifier: v0.3 modifier at block %s not meeting v0.4+ protocol: pindexCurrent nHeight=%d nTime=%u\n", pindexCurrent->GetBlockHash().ToString(), pindexCurrent->nHeight, (unsigned int)pindexCurrent->GetBlockTime());
        }
    }

    // Sort candidate blocks by timestamp
    vector<pair<int64_t, uint256> > vSortedByTimestamp;
    vSortedByTimestamp.reserve(64 * params.nModifierInterval / params.nStakeTargetSpacing);
    int64_t nSelectionInterval = GetStakeModifierSelectionInterval();
    int64_t nSelectionIntervalStart = (pindexPrev->GetBlockTime() / params.nModifierInterval) * params.nModifierInterval - nSelectionInterval;
    const CBlockIndex* pindex = pindexPrev;
    while (pindex && pindex->GetBlockTime() >= nSelectionIntervalStart)
    {
        vSortedByTimestamp.push_back(make_pair(pindex->GetBlockTime(), pindex->GetBlockHash()));
        pindex = pindex->pprev;
    }
    int nHeightFirstCandidate = pindex ? (pindex->nHeight + 1) : 0;

    // Shuffle before sort
    for(int i = vSortedByTimestamp.size() - 1; i > 1; --i)
    	std::swap(vSortedByTimestamp[i], vSortedByTimestamp[GetRand(i)]);

    sort(vSortedByTimestamp.begin(), vSortedByTimestamp.end(), [] (const pair<int64_t, uint256> &a, const pair<int64_t, uint256> &b)
    {
        if (a.first != b.first)
            return a.first < b.first;
//        return b.second < a.second;

        // Timestamp equals - compare block hashes
        const uint32_t *pa = (const uint32_t *)a.second.data();
        const uint32_t *pb = (const uint32_t *)b.second.data();
        int cnt = 256 / 32;
        do {
            --cnt;
            if (pa[cnt] != pb[cnt])
                return pa[cnt] < pb[cnt];
        } while(cnt);
            return false; // Elements are equal

    });

    // Select 64 blocks from candidate blocks to generate stake modifier
    uint64_t nStakeModifierNew = 0;
    int64_t nSelectionIntervalStop = nSelectionIntervalStart;
    map<uint256, const CBlockIndex*> mapSelectedBlocks;
    for (int nRound=0; nRound<min(64, (int)vSortedByTimestamp.size()); nRound++)
    {
        // add an interval section to the current selection round
        nSelectionIntervalStop += GetStakeModifierSelectionIntervalSection(nRound);
        // select a block from the candidates of current round
        if (!SelectBlockFromCandidates(vSortedByTimestamp, mapSelectedBlocks, nSelectionIntervalStop, nStakeModifier, &pindex, chainstate))
            return error("ComputeNextStakeModifier: unable to select block at round %d", nRound);
        // write the entropy bit of the selected block
        nStakeModifierNew |= (((uint64_t)pindex->GetStakeEntropyBit()) << nRound);
        // add the selected block from candidates to selected list
        mapSelectedBlocks.insert(make_pair(pindex->GetBlockHash(), pindex));
        if (gArgs.GetBoolArg("-debug", false) && gArgs.GetBoolArg("-printstakemodifier", false))
            LogPrintf("ComputeNextStakeModifier: selected round %d stop=%s height=%d bit=%d\n",
                nRound, FormatISO8601DateTime(nSelectionIntervalStop), pindex->nHeight, pindex->GetStakeEntropyBit());
    }

    // Print selection map for visualization of the selected blocks
    if (gArgs.GetBoolArg("-debug", false) && gArgs.GetBoolArg("-printstakemodifier", false))
    {
        string strSelectionMap = "";
        // '-' indicates proof-of-work blocks not selected
        strSelectionMap.insert(0, pindexPrev->nHeight - nHeightFirstCandidate + 1, '-');
        pindex = pindexPrev;
        while (pindex && pindex->nHeight >= nHeightFirstCandidate)
        {
            // '=' indicates proof-of-stake blocks not selected
            if (pindex->IsProofOfStake())
                strSelectionMap.replace(pindex->nHeight - nHeightFirstCandidate, 1, "=");
            pindex = pindex->pprev;
        }
        for (const auto& item : mapSelectedBlocks)
        {
            // 'S' indicates selected proof-of-stake blocks
            // 'W' indicates selected proof-of-work blocks
            strSelectionMap.replace(item.second->nHeight - nHeightFirstCandidate, 1, item.second->IsProofOfStake()? "S" : "W");
        }
        LogPrintf("ComputeNextStakeModifier: selection height [%d, %d] map %s\n", nHeightFirstCandidate, pindexPrev->nHeight, strSelectionMap);
    }
    if (gArgs.GetBoolArg("-debug", false))
        LogPrintf("ComputeNextStakeModifier: new modifier=0x%016x time=%s\n", nStakeModifierNew, FormatISO8601DateTime(pindexPrev->GetBlockTime()));

    nStakeModifier = nStakeModifierNew;
    fGeneratedStakeModifier = true;
    return true;
}

// V0.5: Stake modifier used to hash for a stake kernel is chosen as the stake
// modifier that is (nStakeMinAge minus a selection interval) earlier than the
// stake, thus at least a selection interval later than the coin generating the
// kernel, as the generating coin is from at least nStakeMinAge ago.
static bool GetKernelStakeModifierV05(CBlockIndex* pindexPrev, unsigned int nTimeTx, uint64_t& nStakeModifier, int& nStakeModifierHeight, int64_t& nStakeModifierTime, bool fPrintProofOfStake)
{
    const Consensus::Params& params = Params().GetConsensus();
    const CBlockIndex* pindex = pindexPrev;
    nStakeModifierHeight = pindex->nHeight;
    nStakeModifierTime = pindex->GetBlockTime();
    int64_t nStakeModifierSelectionInterval = GetStakeModifierSelectionInterval();

    if (nStakeModifierTime + params.nStakeMinAge - nStakeModifierSelectionInterval <= (int64_t) nTimeTx)
    {
        // Best block is still more than
        // (nStakeMinAge minus a selection interval) older than kernel timestamp
        if (fPrintProofOfStake)
            return error("GetKernelStakeModifier() : best block %s at height %d too old for stake",
                pindex->GetBlockHash().ToString(), pindex->nHeight);
        else
            return false;
    }
    // loop to find the stake modifier earlier by 
    // (nStakeMinAge minus a selection interval)
    while (nStakeModifierTime + params.nStakeMinAge - nStakeModifierSelectionInterval >(int64_t) nTimeTx)
    {
        if (!pindex->pprev)
        {   // reached genesis block; should not happen
            return error("GetKernelStakeModifier() : reached genesis block");
        }
        pindex = pindex->pprev;
        if (pindex->GeneratedStakeModifier())
        {
            nStakeModifierHeight = pindex->nHeight;
            nStakeModifierTime = pindex->GetBlockTime();
        }
    }
    nStakeModifier = pindex->nStakeModifier;
    return true;
}

// V0.3: Stake modifier used to hash for a stake kernel is chosen as the stake
// modifier about a selection interval later than the coin generating the kernel
static bool GetKernelStakeModifierV03(CBlockIndex* pindexPrev, uint256 hashBlockFrom, uint64_t& nStakeModifier, int& nStakeModifierHeight, int64_t& nStakeModifierTime, bool fPrintProofOfStake, Chainstate& chainstate)
{
    const Consensus::Params& params = Params().GetConsensus();
    nStakeModifier = 0;

    const CBlockIndex* pindexFrom;
    {
        LOCK(cs_main);
        pindexFrom = chainstate.m_blockman.LookupBlockIndex(hashBlockFrom);
    }

    if (!pindexFrom)
        return error("GetKernelStakeModifier() : block not indexed");

    nStakeModifierHeight = pindexFrom->nHeight;
    nStakeModifierTime = pindexFrom->GetBlockTime();
    int64_t nStakeModifierSelectionInterval = GetStakeModifierSelectionInterval();


    // we need to iterate index forward but we cannot depend on chainActive.Next()
    // because there is no guarantee that we are checking blocks in active chain.
    // So, we construct a temporary chain that we will iterate over.
    // pindexFrom - this block contains coins that are used to generate PoS
    // pindexPrev - this is a block that is previous to PoS block that we are checking, you can think of it as tip of our chain
    std::vector<CBlockIndex*> tmpChain;
    int32_t nDepth = pindexPrev->nHeight - (pindexFrom->nHeight-1); // -1 is used to also include pindexFrom
    tmpChain.reserve(nDepth);
    CBlockIndex* it = pindexPrev;
    for (int i=1; i<=nDepth && !chainstate.m_chain.Contains(it); i++) {
        tmpChain.push_back(it);
        it = it->pprev;
    }
    std::reverse(tmpChain.begin(), tmpChain.end());
    size_t n = 0;

    const CBlockIndex* pindex = pindexFrom;
    // loop to find the stake modifier later by a selection interval
    while (nStakeModifierTime < pindexFrom->GetBlockTime() + nStakeModifierSelectionInterval)
    {
        const CBlockIndex* old_pindex = pindex;
        pindex = (!tmpChain.empty() && pindex->nHeight >= tmpChain[0]->nHeight - 1)? tmpChain[n++] : chainstate.m_chain.Next(pindex);
        if (n > tmpChain.size() || pindex == NULL) // check if tmpChain[n+1] exists
        {   // reached best block; may happen if node is behind on block chain
            if (fPrintProofOfStake || (old_pindex->GetBlockTime() + params.nStakeMinAge - nStakeModifierSelectionInterval > TicksSinceEpoch<std::chrono::seconds>(GetAdjustedTime())))
                return error("GetKernelStakeModifier() : reached best block %s at height %d from block %s",
                    old_pindex->GetBlockHash().ToString(), old_pindex->nHeight, hashBlockFrom.ToString());
            else
                return false;
        }
        if (pindex->GeneratedStakeModifier())
        {
            nStakeModifierHeight = pindex->nHeight;
            nStakeModifierTime = pindex->GetBlockTime();
        }
    }
    nStakeModifier = pindex->nStakeModifier;
    return true;
}

// Get the stake modifier specified by the protocol to hash for a stake kernel
static bool GetKernelStakeModifier(CBlockIndex* pindexPrev, uint256 hashBlockFrom, unsigned int nTimeTx, uint64_t& nStakeModifier, int& nStakeModifierHeight, int64_t& nStakeModifierTime, bool fPrintProofOfStake, Chainstate& chainstate)
{
    if (IsProtocolV05(nTimeTx))
        return GetKernelStakeModifierV05(pindexPrev, nTimeTx, nStakeModifier, nStakeModifierHeight, nStakeModifierTime, fPrintProofOfStake);
    else
        return GetKernelStakeModifierV03(pindexPrev, hashBlockFrom, nStakeModifier, nStakeModifierHeight, nStakeModifierTime, fPrintProofOfStake, chainstate);
}

// peercoin kernel protocol
// coinstake must meet hash target according to the protocol:
// kernel (input 0) must meet the formula
//     hash(nStakeModifier + txPrev.block.nTime + txPrev.offset + txPrev.nTime + txPrev.vout.n + nTime) < bnTarget * nCoinDayWeight
// this ensures that the chance of getting a coinstake is proportional to the
// amount of coin age one owns.
// The reason this hash is chosen is the following:
//   nStakeModifier: 
//       (v0.5) uses dynamic stake modifier around 21 days before the kernel,
//              versus static stake modifier about 9 days after the staked
//              coin (txPrev) used in v0.3
//       (v0.3) scrambles computation to make it very difficult to precompute
//              future proof-of-stake at the time of the coin's confirmation
//       (v0.2) nBits (deprecated): encodes all past block timestamps
//   txPrev.block.nTime: prevent nodes from guessing a good timestamp to
//                       generate transaction for future advantage
//   txPrev.offset: offset of txPrev inside block, to reduce the chance of 
//                  nodes generating coinstake at the same time
//   txPrev.nTime: reduce the chance of nodes generating coinstake at the same
//                 time
//   txPrev.vout.n: output number of txPrev, to reduce the chance of nodes
//                  generating coinstake at the same time
//   block/tx hash should not be used here as they can be generated in vast
//   quantities so as to generate blocks faster, degrading the system back into
//   a proof-of-work situation.
//
bool CheckStakeKernelHash(unsigned int nBits, CBlockIndex* pindexPrev, const CBlockHeader& blockFrom, unsigned int nTxPrevOffset, const CTransactionRef& txPrev, const COutPoint& prevout, unsigned int nTimeTx, uint256& hashProofOfStake, bool fPrintProofOfStake, Chainstate& chainstate)
{
    const Consensus::Params& params = Params().GetConsensus();
    unsigned int nTimeBlockFrom = blockFrom.GetBlockTime();

    if (nTimeTx < (txPrev->nTime? txPrev->nTime : nTimeBlockFrom))  // Transaction timestamp violation
        return error("CheckStakeKernelHash() : nTime violation");

    if (nTimeBlockFrom + params.nStakeMinAge > nTimeTx) // Min age requirement
        return error("CheckStakeKernelHash() : min age violation");

    CBigNum bnTargetPerCoinDay;
    bnTargetPerCoinDay.SetCompact(nBits);
    int64_t nValueIn = txPrev->vout[prevout.n].nValue;
    // v0.3 protocol kernel hash weight starts from 0 at the 30-day min age
    // this change increases active coins participating the hash and helps
    // to secure the network when proof-of-stake difficulty is low
//    int64_t nTimeWeight = min((int64_t)nTimeTx - (txPrev->nTime? txPrev->nTime : nTimeBlockFrom), params.nStakeMaxAge) - (IsProtocolV03(nTimeTx)? params.nStakeMinAge : 0)
    int64_t nTimeWeight = GetMagiWeightV2(nValueIn, (txPrev->nTime? txPrev->nTime : nTimeBlockFrom), nTimeTx);
    CBigNum bnCoinDayWeight = CBigNum(nValueIn) * nTimeWeight / COIN / (24 * 60 * 60);
    // Calculate hash
    CDataStream ss(SER_GETHASH, 0);
    uint64_t nStakeModifier = 0;
    int nStakeModifierHeight = 0;
    int64_t nStakeModifierTime = 0;
    if (IsProtocolV03(nTimeTx))  // v0.3 protocol
    {
        if (!GetKernelStakeModifier(pindexPrev, blockFrom.GetHash(), nTimeTx, nStakeModifier, nStakeModifierHeight, nStakeModifierTime, fPrintProofOfStake, chainstate))
            return false;
        ss << nStakeModifier;
    }
    else // v0.2 protocol
    {
        ss << nBits;
    }

    ss << nTimeBlockFrom << nTxPrevOffset << (txPrev->nTime? txPrev->nTime : nTimeBlockFrom) << prevout.n << nTimeTx;
    hashProofOfStake = Hash(ss);
    if (fPrintProofOfStake)
    {
        if (IsProtocolV03(nTimeTx)) {
            LOCK(cs_main);
            const CBlockIndex* pindexTmp = chainstate.m_blockman.LookupBlockIndex(blockFrom.GetHash());
            LogPrintf("CheckStakeKernelHash() : using modifier 0x%016x at height=%d timestamp=%s for block from height=%d timestamp=%s\n",
                nStakeModifier, nStakeModifierHeight,
                FormatISO8601DateTime(nStakeModifierTime),
                pindexTmp->nHeight,
                FormatISO8601DateTime(blockFrom.GetBlockTime()));
        }
        LogPrintf("CheckStakeKernelHash() : check protocol=%s modifier=0x%016x nTimeBlockFrom=%u nTxPrevOffset=%u nTimeTxPrev=%u nPrevout=%u nTimeTx=%u hashProof=%s\n",
            IsProtocolV05(nTimeTx)? "0.5" : (IsProtocolV03(nTimeTx)? "0.3" : "0.2"),
            IsProtocolV03(nTimeTx)? nStakeModifier : (uint64_t) nBits,
            nTimeBlockFrom, nTxPrevOffset, (txPrev->nTime? txPrev->nTime : nTimeBlockFrom), prevout.n, nTimeTx,
            hashProofOfStake.ToString());
    }

    // Now check if proof-of-stake hash meets target protocol
    if (CBigNum(hashProofOfStake) > bnCoinDayWeight * bnTargetPerCoinDay)
        return false;
    if (gArgs.GetBoolArg("-debug", false) && !fPrintProofOfStake)
    {
        if (IsProtocolV03(nTimeTx)) {
            LOCK(cs_main);
            const CBlockIndex* pindexTmp = chainstate.m_blockman.LookupBlockIndex(blockFrom.GetHash());
            LogPrintf("CheckStakeKernelHash() : using modifier 0x%016x at height=%d timestamp=%s for block from height=%d timestamp=%s\n",
                nStakeModifier, nStakeModifierHeight, 
                FormatISO8601DateTime(nStakeModifierTime),
                pindexTmp->nHeight,
                FormatISO8601DateTime(blockFrom.GetBlockTime()));
        }
        LogPrintf("CheckStakeKernelHash() : pass protocol=%s modifier=0x%016x nTimeBlockFrom=%u nTxPrevOffset=%u nTimeTxPrev=%u nPrevout=%u nTimeTx=%u hashProof=%s\n",
            IsProtocolV03(nTimeTx)? "0.3" : "0.2",
            IsProtocolV03(nTimeTx)? nStakeModifier : (uint64_t) nBits,
            nTimeBlockFrom, nTxPrevOffset, (txPrev->nTime? txPrev->nTime : nTimeBlockFrom), prevout.n, nTimeTx,
            hashProofOfStake.ToString());
    }
    return true;
}

// Check kernel hash target and coinstake signature
bool CheckProofOfStake(BlockValidationState &state, CBlockIndex* pindexPrev, const CTransactionRef& tx, unsigned int nBits, uint256& hashProofOfStake, unsigned int nTimeTx, Chainstate& chainstate)
{
    if (!tx->IsCoinStake())
        return error("CheckProofOfStake() : called on non-coinstake %s", tx->GetHash().ToString());

    // Kernel (input 0) must match the stake hash target per coin age (nBits)
    const CTxIn& txin = tx->vin[0];

    // Transaction index is required to get to block header
    if (!g_txindex)
        return error("CheckProofOfStake() : transaction index not available");

    // Get transaction index for the previous transaction
    CDiskTxPos postx;
    if (!g_txindex->FindTxPosition(txin.prevout.hash, postx))
        return error("CheckProofOfStake() : tx index not found");  // tx index not found

    // Read txPrev and header of its block
    CBlockHeader header;
    CTransactionRef txPrev;
    auto it = g_txindex->cachedTxs.find(txin.prevout.hash);
    if (it != g_txindex->cachedTxs.end()) {
        header = it->second.first;
        txPrev = it->second.second;
    } else {
        CAutoFile file(node::OpenBlockFile(postx, true), SER_DISK, CLIENT_VERSION);
        try {
            file >> header;
            fseek(file.Get(), postx.nTxOffset, SEEK_CUR);
            file >> txPrev;
        } catch (std::exception &e) {
            return error("%s() : deserialize or I/O error in CheckProofOfStake()", __PRETTY_FUNCTION__);
        }
        //g_txindex->cachedTxs[txin.prevout.hash] = std::pair(header,txPrev);
    }

    if (txPrev->GetHash() != txin.prevout.hash)
        return error("%s() : txid mismatch in CheckProofOfStake()", __PRETTY_FUNCTION__);

    // Verify signature
    {
        int nIn = 0;
        const CTxOut& prevOut = txPrev->vout[tx->vin[nIn].prevout.n];
        TransactionSignatureChecker checker(&(*tx), nIn, prevOut.nValue, PrecomputedTransactionData(*tx), MissingDataBehavior(1));

        if (!VerifyScript(tx->vin[nIn].scriptSig, prevOut.scriptPubKey, &(tx->vin[nIn].scriptWitness), SCRIPT_VERIFY_P2SH, checker, nullptr))
            return state.Invalid(BlockValidationResult::BLOCK_CONSENSUS, "invalid-pos-script", strprintf("%s: VerifyScript failed on coinstake %s", __func__, tx->GetHash().ToString()));
    }

    if (!CheckStakeKernelHash(nBits, pindexPrev, header, postx.nTxOffset + CBlockHeader::NORMAL_SERIALIZE_SIZE, txPrev, txin.prevout, nTimeTx, hashProofOfStake, gArgs.GetBoolArg("-debug", false), chainstate))
        return state.Invalid(BlockValidationResult::BLOCK_CONSENSUS, "check-kernel-failed", strprintf("CheckProofOfStake() : INFO: check kernel failed on coinstake %s, hashProof=%s", tx->GetHash().ToString(), hashProofOfStake.ToString())); // may occur during initial download or if behind on block chain sync

    return true;
}

// Check whether the coinstake timestamp meets protocol
bool CheckCoinStakeTimestamp(int64_t nTimeBlock, int64_t nTimeTx)
{
    if (IsProtocolV03(nTimeTx))  // v0.3 protocol
        return (nTimeBlock == nTimeTx);
    else // v0.2 protocol
        return ((nTimeTx <= nTimeBlock) && (nTimeBlock <= nTimeTx + MAX_FUTURE_BLOCK_TIME_PREV9));
}

// Get stake modifier checksum
unsigned int GetStakeModifierChecksum(const CBlockIndex* pindex)
{
    assert (pindex->pprev || pindex->GetBlockHash() == Params().GetConsensus().hashGenesisBlock);
    // Hash previous checksum with flags, hashProofOfStake and nStakeModifier
    CDataStream ss(SER_GETHASH, 0);
    if (pindex->pprev)
        ss << pindex->pprev->nStakeModifierChecksum;
    ss << pindex->nFlags << pindex->hashProofOfStake << pindex->nStakeModifier;
    arith_uint256 hashChecksum = UintToArith256(Hash(ss));
    hashChecksum >>= (256 - 32);
    return hashChecksum.GetLow64();
}

// Check stake modifier hard checkpoints
bool CheckStakeModifierCheckpoints(int nHeight, unsigned int nStakeModifierChecksum)
{
    bool fTestNet = Params().NetworkIDString() == CBaseChainParams::TESTNET;
    if (fTestNet && mapStakeModifierTestnetCheckpoints.count(nHeight))
        return nStakeModifierChecksum == mapStakeModifierTestnetCheckpoints[nHeight];

    if (!fTestNet && mapStakeModifierCheckpoints.count(nHeight))
        return nStakeModifierChecksum == mapStakeModifierCheckpoints[nHeight];

    return true;
}

bool IsSuperMajority(int minVersion, const CBlockIndex* pstart, unsigned int nRequired, unsigned int nToCheck)
{
    return (HowSuperMajority(minVersion, pstart, nRequired, nToCheck) >= nRequired);
}

unsigned int HowSuperMajority(int minVersion, const CBlockIndex* pstart, unsigned int nRequired, unsigned int nToCheck)
{
    unsigned int nFound = 0;
    for (unsigned int i = 0; i < nToCheck && nFound < nRequired && pstart != NULL; pstart = pstart->pprev )
    {
        if (!pstart->IsProofOfStake())
            continue;

        if (pstart->nVersion >= minVersion)
            ++nFound;

        i++;
    }
    return nFound;
}

// peercoin: entropy bit for stake modifier if chosen by modifier
unsigned int GetStakeEntropyBit(const CBlock& block)
{
    unsigned int nEntropyBit = 0;
    if (IsProtocolV04(block.nTime))
    {
        nEntropyBit = UintToArith256(block.GetHash()).GetLow64() & 1llu;// last bit of block hash
        if (gArgs.GetBoolArg("-printstakemodifier", false))
            LogPrintf("GetStakeEntropyBit(v0.4+): nTime=%u hashBlock=%s entropybit=%d\n", block.nTime, block.GetHash().ToString(), nEntropyBit);
    }
    else
    {
        // old protocol for entropy bit pre v0.4
        uint160 hashSig = Hash160(block.vchBlockSig);
        if (gArgs.GetBoolArg("-printstakemodifier", false))
            LogPrintf("GetStakeEntropyBit(v0.3): nTime=%u hashSig=%s", block.nTime, hashSig.ToString());
        nEntropyBit = hashSig.data()[19] >> 7;  // take the first bit of the hash
        if (gArgs.GetBoolArg("-printstakemodifier", false))
            LogPrintf(" entropybit=%d\n", nEntropyBit);
    }
    return nEntropyBit;
}

