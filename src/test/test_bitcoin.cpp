// Copyright (c) 2011-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "test_bitcoin.h"

#include "chainparams.h"
#include "consensus/consensus.h"
#include "consensus/validation.h"
#include "crypto/sha256.h"
#include "fs.h"
#include "key.h"
#include "validation.h"
#include "miner.h"
#include "net_processing.h"
#include "pubkey.h"
#include "random.h"
#include "txdb.h"
#include "txmempool.h"
#include "ui_interface.h"
#include "streams.h"
#include "rpc/server.h"
#include "rpc/register.h"
#include "script/sigcache.h"

#include "prime/prime.h"

#include <memory>

uint256 insecure_rand_seed = GetRandHash();
FastRandomContext insecure_rand_ctx(insecure_rand_seed);

extern bool fPrintToConsole;
extern void noui_connect();

BasicTestingSetup::BasicTestingSetup(const std::string& chainName)
{
        SHA256AutoDetect();
        RandomInit();
        ECC_Start();
        SetupEnvironment();
        SetupNetworking();
        InitSignatureCache();
        InitScriptExecutionCache();
        fPrintToDebugLog = false; // don't want to write to debug.log file
        fCheckBlockIndex = true;
        SelectParams(chainName);
        noui_connect();
}

BasicTestingSetup::~BasicTestingSetup()
{
        ECC_Stop();
}

TestingSetup::TestingSetup(const std::string& chainName) : BasicTestingSetup(chainName)
{
    const CChainParams& chainparams = Params();
        // Ideally we'd move all the RPC tests to the functional testing framework
        // instead of unit tests, but for now we need these here.

        RegisterAllCoreRPCCommands(tableRPC);
        ClearDatadirCache();
        pathTemp = fs::temp_directory_path() / strprintf("test_bitcoin_%lu_%i", (unsigned long)GetTime(), (int)(InsecureRandRange(100000)));
        fs::create_directories(pathTemp);
        gArgs.ForceSetArg("-datadir", pathTemp.string());

        // Note that because we don't bother running a scheduler thread here,
        // callbacks via CValidationInterface are unreliable, but that's OK,
        // our unit tests aren't testing multiple parts of the code at once.
        GetMainSignals().RegisterBackgroundSignalScheduler(scheduler);

        mempool.setSanityCheck(1.0);
        pblocktree = new CBlockTreeDB(1 << 20, true);
        pcoinsdbview = new CCoinsViewDB(1 << 23, true);
        pcoinsTip = new CCoinsViewCache(pcoinsdbview);
        if (!LoadGenesisBlock(chainparams)) {
            throw std::runtime_error("LoadGenesisBlock failed.");
        }
        {
            CValidationState state;
            if (!ActivateBestChain(state, chainparams)) {
                throw std::runtime_error("ActivateBestChain failed.");
            }
        }
        nScriptCheckThreads = 3;
        for (int i=0; i < nScriptCheckThreads-1; i++)
            threadGroup.create_thread(&ThreadScriptCheck);
        g_connman = std::unique_ptr<CConnman>(new CConnman(0x1337, 0x1337)); // Deterministic randomness for tests.
        connman = g_connman.get();
        peerLogic.reset(new PeerLogicValidation(connman));
}

TestingSetup::~TestingSetup()
{
        threadGroup.interrupt_all();
        threadGroup.join_all();
        GetMainSignals().FlushBackgroundCallbacks();
        GetMainSignals().UnregisterBackgroundSignalScheduler();
        g_connman.reset();
        peerLogic.reset();
        UnloadBlockIndex();
        delete pcoinsTip;
        delete pcoinsdbview;
        delete pblocktree;
        fs::remove_all(pathTemp);
}

TestChain100Setup::TestChain100Setup() : TestingSetup(CBaseChainParams::REGTEST)
{
    //DATACOIN ADDED
    // Primecoin: Generate prime table when starting up
    GeneratePrimeTable();
    InitPrimeMiner();
			
    // Generate a 100-block chain:
    coinbaseKey.MakeNewKey(true);
    CScript scriptPubKey = CScript() <<  ToByteVector(coinbaseKey.GetPubKey()) << OP_CHECKSIG;
    for (int i = 0; i < COINBASE_MATURITY; i++)
    {
        std::vector<CMutableTransaction> noTxns;
        //std::cerr << "TRY i " << i << "\n";
        CBlock b = CreateAndProcessBlock(noTxns, scriptPubKey);
        coinbaseTxns.push_back(*b.vtx[0]);
    }
}

//
// Create a new block with just given transactions, coinbase paying to
// scriptPubKey, and try to add it to the current chain.
//
CBlock
TestChain100Setup::CreateAndProcessBlock(const std::vector<CMutableTransaction>& txns, const CScript& scriptPubKey)
{
	const CChainParams& chainparams = Params();
    std::unique_ptr<CBlockTemplate> pblocktemplate = BlockAssembler(chainparams).CreateNewBlock(scriptPubKey);
    CBlock& block = pblocktemplate->block;

    // Replace mempool-selected txns with just coinbase plus passed-in txns:
    block.vtx.resize(1);
    for (const CMutableTransaction& tx : txns)
        block.vtx.push_back(MakeTransactionRef(tx));
    // IncrementExtraNonce creates a valid coinbase and merkleRoot
    unsigned int extraNonce = 0;
    IncrementExtraNonce(&block, chainActive.Tip(), extraNonce);

    //DATACOIN MINER
    //unsigned int nChainType;
    //unsigned int nChainLength;
    //while (!CheckProofOfWork(block.GetHeaderHash(), block.nBits, Params().GetConsensus(), bnProbablePrime, nChainType, nChainLength)) ++block.nNonce;
	while (!CheckProofOfWork(block.GetHeaderHash(), block.nBits, Params().GetConsensus(), block.bnPrimeChainMultiplier, block.nPrimeChainType, block.nPrimeChainLength)) ++block.nNonce;

    std::shared_ptr<const CBlock> shared_pblock = std::make_shared<const CBlock>(block);
    if (!ProcessNewBlock(chainparams, shared_pblock, true, nullptr)) 
    {
        //std::cerr << "ERROR in ProcessNewBlock\n";
    }
    //else {std::cerr << "---ACCEPTED---\n";}

    CBlock result = block;
    return result;
}

TestChain100Setup::~TestChain100Setup()
{
}


CTxMemPoolEntry TestMemPoolEntryHelper::FromTx(const CMutableTransaction &tx) {
    CTransaction txn(tx);
    return FromTx(txn);
}

CTxMemPoolEntry TestMemPoolEntryHelper::FromTx(const CTransaction &txn) {
    return CTxMemPoolEntry(MakeTransactionRef(txn), nFee, nTime, nHeight,
                           spendsCoinbase, sigOpCost, lp);
}

/** //DATACOIN CHANGED
 * @returns a real block (2c69c597d88be01c544c38c3dfd97d08d0bf19353ba18e4abc48d251f2aa72ee)
 *      with 7 txs.
 */
CBlock getBlock13b8a()
{
    CBlock block;
    CDataStream stream(ParseHex("020000002bfb3b9a1dd788b4496c64211015ce28e0c4619a0d1beef1f96b3be7d8f5b378ee792e7f3ad0fbc3780c9337fb7c2d75a6dcd2c28e8e76348e29d471e28ca3843b78e55ae2a3da092e5b62490890b959615cdca0100701000000010000000000000000000000000000000000000000000000000000000000000000ffffffff0e03dd8124010200062f503253482fffffffff0180cc0f3f00000000232102c2914a1176cf1afc5087c6ced7c9cf6638f20804e35a97a834572f4fbacf5dd1ac00000000000100000001cd4a688fbe36d6237c84daef35b6f0709b723c246cd39f8f4136cfe81b386f96000000004948304502200e4bebe28b7a4da72f7f282a3b69b1cec3eb95e707389c32d7e0f8cd14619b8b0221008b14b4dd7ede35c263ad60ef9144fc1919864cca2058d2c5581d17891474c2e101ffffffff02867f4934000000001976a914d972c0b931e5b0453d005d99c429f5838c5d68ea88ac7a6e4508000000001976a914ac6f464987138d73504977166bfcbe70d65c304288ac00000000000100000001430c08346b0e101ccb7fbfc2241a3b56b401f6f2c5395e8d20bef106bbc6a131000000004948304502210082bc0fa9ecb30ca03a057f16d7f29d565ba3b5ed7c7446fc12a010597df8dc7b022006dd861bc2c898459652fcba96642b928005c209996521830b405afff053439601ffffffff02958a8808000000001976a914ac6f464987138d73504977166bfcbe70d65c304288ac6b630634000000001976a914021ad4af372e5f4c84ae5b6be60534f2d8ee65fc88ac00000000000100000001cfd1a851e792fc64f3ba0369917c3cceed45fa1ed3ec713d246b901edc82f69c000000004948304502204397f0f10f8eba5cb568acfecad7b1dd02d1cf702cca95f4c467a76e462162af02210084e38ac83bd96f1a6c7ec71d89d957603209f974a472065aeeb42f005c2a9cde01ffffffff02a69b6536000000001976a91418514d8451d4c15032ff735692043177adca0adc88ac5a522906000000001976a914ac6f464987138d73504977166bfcbe70d65c304288ac00000000000100000001607ca9880dc24dcc357ae584863c4c77c2321aace60e71dc589b14f43bf367e90000000048473044022017da2e628e7410d265e332e5788e820628952dd420d1ba5ebccbbb4507eedf58022077d165514b5d6f3f2ea92b4558602b54645a4966bf7ec4c284e89ad4d2da19b301ffffffff026a409e3b000000001976a914e142f3e79635f19ea88ffcf6eae658f46c2f497b88ac96adf000000000001976a914f12cc2472f062cf76879a21ab83e5ba309bd594588ac000000000001000000066f61e9761e16cc276cd106fda07c84ae6e69d7dc1ad9029f2f86ff9b73db87770000000048473044022011713ad4e5347db5f606c58087c4e30ade853348bf204a86ea09eb3129720904022059ce0bf168e3af6ee684d4ba6aef23a69cd4dc79c5a131da214c6b2a1968f6a801ffffffff1759a8a28eb418d447fc91a6c8653ac3c2662e2a7fd1c2ee8a845832e64b1bcf0000000049483045022100c177a21875b1fbc2c397cf624ba9bc48acb31ac8eb549ff75613e9f6807a7f350220124d87950f3e0fe1b4c2d1d081b0cfd66a1b778aaf1c27e41ca0b914a5a7a1a301ffffffff780245857899914071228fdb5993fea8bdf4dd09cd4fd937655883d1b0a92a31000000004a49304602210090d32e6531e834cb40d12eb893796d2fa811a8be9ea9ce2109e3e4a38ab3db36022100931705ad6550f902a6a8a89a4157ced3f485e743077f35808909e75174dcc8d401ffffffff8fb8d12cf454b15bff2d197fcb5d25ad00da56ce41e5feed039f52c516915962000000004a4930460221009982fa810b7a1d42931ed264789d6cea8f09eca4dfccf8972a86eb8ca41015ee022100b72b34d15637147d589a61d350f3b8d03ecd94943982c1c7bedab66fc1e16ed401ffffffff64e4ece2bda3ca27c0b3f3e3061d8881dbb14d4772eb1a58a194395e0097d156010000006a47304402205319c42bd116442cecf0106bba735098cc55a0997a71d3e4d517a250e1f60e6202203081e2feea2c2e962fbe370e0e600f6169e7b419695f0c9cb53961dea5756a40012102f7925f4aad03b336cc9942426c3b16e779198fa015aa09e4b439a1050d625aeeffffffffc9f19af326c89628a3429b0ade84599036df88360fb6c592718fbe32e39f1557010000006a47304402201421f91bb5f613b1ebdfb696728b497c78483426b56b3f8cc3d89f708f39c59902207e3fe4088497d060763f8fec00a970eb9bb1d0e3596f64975dc773434cc5613c012102be34ae9a965c56c539f4a6812ac3eba3f02cb726c162d061e29ab08d44017722ffffffff024f12e501000000001976a9149373c063bdfa74ed4ab78de6289bf2d777417de488acf72c892a010000001976a914971b60d20a6cdd97f0dcb3feaf586b482bd2c56f88ac0000000000010000000180133aa762c5ae70f53d9d881d11c3a76efe9736233121319cb0506834b5182b010000006b483045022100ace7363878d527040ef0c05dbb1064aca3550cb303545515197c904bb459a45b0220714f5b01d1c03b2087ca5ece927b03bdcb7dd87fd4cc880a0d0b387169e0ce580121021c0d7044ff47dfc66612ebaf7a1b4313fa62a608204f0846edc82e30c8accc4bffffffff027ced3d1b000000001976a9145257e8b6cdf278a1a28f5ae911ac2ff60a71e20988ac25e63a0d000000001976a914ac6f464987138d73504977166bfcbe70d65c304288ac0000000000"), SER_NETWORK, PROTOCOL_VERSION);
    stream >> block;
    return block;
}
