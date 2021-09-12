// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "Helpers.hpp"  // IWYU pragma: associated

#include <boost/move/algo/detail/set_difference.hpp>
#include <boost/move/algo/move.hpp>
#include <chrono>
#include <future>
#include <iosfwd>
#include <map>
#include <mutex>
#include <stdexcept>
#include <utility>

#include "opentxs/Pimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/socket/Subscribe.hpp"

namespace zmq = ot::network::zeromq;

namespace ottest
{
struct Listener::Imp {
    auto get(std::size_t index) noexcept(false) -> const Message&
    {
        auto future = data_.future(index);
        static constexpr auto timeout = std::chrono::minutes{5};
        using Status = std::future_status;

        if (const auto s = future.wait_for(timeout); s != Status::ready) {
            throw std::runtime_error{"timeout"};
        }

        return future.get();
    }

    Imp(const ot::api::Core& api, const std::string& endpoint)
        : lock_()
        , data_()
        , counter_(-1)
        , cb_(zmq::ListenCallback::Factory([this](auto& in) { cb(in); }))
        , socket_([&] {
            auto out = api.Network().ZeroMQ().SubscribeSocket(cb_);

            if (false == out->Start(endpoint)) {
                throw std::runtime_error{"failed to connect to endpoint"};
            }

            return out;
        }())
    {
    }

private:
    using Promise = std::promise<ot::OTZMQMessage>;
    using Future = std::shared_future<ot::OTZMQMessage>;

    struct Task {
        Promise promise_;
        Future future_;

        Task() noexcept
            : promise_()
            , future_(promise_.get_future())
        {
        }
        Task(Task&& rhs) noexcept
            : promise_(std::move(rhs.promise_))
            , future_(std::move(rhs.future_))
        {
        }

    private:
        Task(const Task&) = delete;

        auto operator=(const Task&) -> Task& = delete;
        auto operator=(Task&&) -> Task& = delete;
    };

    struct Promises {
        auto future(std::size_t index) noexcept -> Future
        {
            auto lock = ot::Lock{lock_};

            return map_[index].future_;
        }
        auto promise(std::size_t index) noexcept -> Promise&
        {
            auto lock = ot::Lock{lock_};

            return map_[index].promise_;
        }

    private:
        mutable std::mutex lock_{};
        std::map<std::size_t, Task> map_{};
    };

    mutable std::mutex lock_;
    Promises data_;
    std::ptrdiff_t counter_;
    ot::OTZMQListenCallback cb_;
    ot::OTZMQSubscribeSocket socket_;

    auto cb(Message& in) noexcept -> void
    {
        data_.promise(++counter_).set_value(std::move(in));
    }
};

Listener::Listener(const ot::api::Core& api, const std::string& endpoint)
    : imp_(std::make_unique<Imp>(api, endpoint))
{
}

auto Listener::get(std::size_t index) noexcept(false) -> const Message&
{
    return imp_->get(index);
}

Listener::~Listener() = default;

const boost::container::flat_map<ot::blockchain::Type, ChainVector>
    genesis_block_data_{
        {ot::blockchain::Type::UnitTest,
         {"01000000000000000000000000000000000000000000000000000000000000000000"
          "00003ba3edfd7a7b12b27ac72c3e67768f617fc81bc3888a51323a9fb8aa4b1e5e4a"
          "dae5494dffff7f200200000001010000000100000000000000000000000000000000"
          "00000000000000000000000000000000ffffffff4d04ffff001d0104455468652054"
          "696d65732030332f4a616e2f32303039204368616e63656c6c6f72206f6e20627269"
          "6e6b206f66207365636f6e64206261696c6f757420666f722062616e6b73ffffffff"
          "0100f2052a01000000434104678afdb0fe5548271967f1a67130b7105cd6a828e039"
          "09a67962e0ea1f61deb649f6bc3f4cef38c4f35504e51ec112de5c384df7ba0b8d57"
          "8a4c702b6bf11d5fac00000000",
          {
              {ot::blockchain::filter::Type::Basic_BIP158,
               {"014756c0",
                "2b5adc66021d5c775f630efd91518cf6ce3e9f525bbf54d9f0d709451e305e"
                "48"}},
              {ot::blockchain::filter::Type::Basic_BCHVariant,
               {"014756c0",
                "2b5adc66021d5c775f630efd91518cf6ce3e9f525bbf54d9f0d709451e305e"
                "48"}},
              {ot::blockchain::filter::Type::ES,
               {"042547f61f786604db036044c4f7f36fe0",
                "5e0aa302450f931bc2e4fab27632231a06964277ea8dfcdd93c19149a24fe7"
                "88"}},
          }}},
        {ot::blockchain::Type::Bitcoin,
         {"01000000000000000000000000000000000000000000000000000000000000000000"
          "00003ba3edfd7a7b12b27ac72c3e67768f617fc81bc3888a51323a9fb8aa4b1e5e4a"
          "29ab5f49ffff001d1dac2b7c01010000000100000000000000000000000000000000"
          "00000000000000000000000000000000ffffffff4d04ffff001d0104455468652054"
          "696d65732030332f4a616e2f32303039204368616e63656c6c6f72206f6e20627269"
          "6e6b206f66207365636f6e64206261696c6f757420666f722062616e6b73ffffffff"
          "0100f2052a01000000434104678afdb0fe5548271967f1a67130b7105cd6a828e039"
          "09a67962e0ea1f61deb649f6bc3f4cef38c4f35504e51ec112de5c384df7ba0b8d57"
          "8a4c702b6bf11d5fac00000000",
          {
              {ot::blockchain::filter::Type::Basic_BIP158,
               {"017fa880",
                "9f3c30f0c37fb977cf3e1a3173c631e8ff119ad3088b6f5b2bced0802139c2"
                "02"}},
              {ot::blockchain::filter::Type::ES,
               {"049dc75e903561289b0029337bcf4e6720",
                "fad52acc389a391c1d6d94e8984fe77323fbda24fb31299b88635d7bee0278"
                "e8"}},
          }}},
        {ot::blockchain::Type::Bitcoin_testnet3,
         {"01000000000000000000000000000000000000000000000000000000000000000000"
          "00003ba3edfd7a7b12b27ac72c3e67768f617fc81bc3888a51323a9fb8aa4b1e5e4a"
          "dae5494dffff001d1aa4ae1801010000000100000000000000000000000000000000"
          "00000000000000000000000000000000ffffffff4d04ffff001d0104455468652054"
          "696d65732030332f4a616e2f32303039204368616e63656c6c6f72206f6e20627269"
          "6e6b206f66207365636f6e64206261696c6f757420666f722062616e6b73ffffffff"
          "0100f2052a01000000434104678afdb0fe5548271967f1a67130b7105cd6a828e039"
          "09a67962e0ea1f61deb649f6bc3f4cef38c4f35504e51ec112de5c384df7ba0b8d57"
          "8a4c702b6bf11d5fac00000000",
          {
              {ot::blockchain::filter::Type::Basic_BIP158,
               {"019dfca8",
                "50b781aed7b7129012a6d20e2d040027937f3affaee573779908ebb7794558"
                "21"}},
              {ot::blockchain::filter::Type::ES,
               {"04e2f5880d851afd74c662d38d49e29130",
                "995cfe5d055c9158c5a388b71fb2ddbe292c9ca2d30dca91359d8cbbe4603e"
                "02"}},
          }}},
        {ot::blockchain::Type::BitcoinCash,
         {"01000000000000000000000000000000000000000000000000000000000000000000"
          "00003ba3edfd7a7b12b27ac72c3e67768f617fc81bc3888a51323a9fb8aa4b1e5e4a"
          "29ab5f49ffff001d1dac2b7c01010000000100000000000000000000000000000000"
          "00000000000000000000000000000000ffffffff4d04ffff001d0104455468652054"
          "696d65732030332f4a616e2f32303039204368616e63656c6c6f72206f6e20627269"
          "6e6b206f66207365636f6e64206261696c6f757420666f722062616e6b73ffffffff"
          "0100f2052a01000000434104678afdb0fe5548271967f1a67130b7105cd6a828e039"
          "09a67962e0ea1f61deb649f6bc3f4cef38c4f35504e51ec112de5c384df7ba0b8d57"
          "8a4c702b6bf11d5fac00000000",
          {
              {ot::blockchain::filter::Type::Basic_BCHVariant,
               {"017fa880",
                "9f3c30f0c37fb977cf3e1a3173c631e8ff119ad3088b6f5b2bced0802139c2"
                "02"}},
              {ot::blockchain::filter::Type::ES,
               {"049dc75e903561289b0029337bcf4e6720",
                "fad52acc389a391c1d6d94e8984fe77323fbda24fb31299b88635d7bee0278"
                "e8"}},
          }}},
        {ot::blockchain::Type::BitcoinCash_testnet3,
         {"01000000000000000000000000000000000000000000000000000000000000000000"
          "00003ba3edfd7a7b12b27ac72c3e67768f617fc81bc3888a51323a9fb8aa4b1e5e4a"
          "dae5494dffff001d1aa4ae1801010000000100000000000000000000000000000000"
          "00000000000000000000000000000000ffffffff4d04ffff001d0104455468652054"
          "696d65732030332f4a616e2f32303039204368616e63656c6c6f72206f6e20627269"
          "6e6b206f66207365636f6e64206261696c6f757420666f722062616e6b73ffffffff"
          "0100f2052a01000000434104678afdb0fe5548271967f1a67130b7105cd6a828e039"
          "09a67962e0ea1f61deb649f6bc3f4cef38c4f35504e51ec112de5c384df7ba0b8d57"
          "8a4c702b6bf11d5fac00000000",
          {
              {ot::blockchain::filter::Type::Basic_BCHVariant,
               {"019dfca8",
                "50b781aed7b7129012a6d20e2d040027937f3affaee573779908ebb7794558"
                "21"}},
              {ot::blockchain::filter::Type::ES,
               {"04e2f5880d851afd74c662d38d49e29130",
                "995cfe5d055c9158c5a388b71fb2ddbe292c9ca2d30dca91359d8cbbe4603e"
                "02"}},
          }}},
        {ot::blockchain::Type::Litecoin,
         {"01000000000000000000000000000000000000000000000000000000000000000000"
          "0000d9ced4ed1130f7b7faad9be25323ffafa33232a17c3edf6cfd97bee6bafbdd97"
          "b9aa8e4ef0ff0f1ecd513f7c01010000000100000000000000000000000000000000"
          "00000000000000000000000000000000ffffffff4804ffff001d0104404e59205469"
          "6d65732030352f4f63742f32303131205374657665204a6f62732c204170706c65e2"
          "80997320566973696f6e6172792c2044696573206174203536ffffffff0100f2052a"
          "010000004341040184710fa689ad5023690c80f3a49c8f13f8d45b8c857fbcbc8bc4"
          "a8e4d3eb4b10f4d4604fa08dce601aaf0f470216fe1b51850b4acf21b179c45070ac"
          "7b03a9ac00000000",
          {
              {ot::blockchain::filter::Type::Basic_BIP158,
               {"019e8738",
                "8aa75530308cf8247a151c37c24e7aaa281ae3b5cecedb581aacb3a0d07c24"
                "51"}},
              {ot::blockchain::filter::Type::ES,
               {"049de896b2cc882671e81f336fdf119b00",
                "23b8dae37cf04c8a278bd50bcbcf23a03051ea902f67c4760eb35be96d4283"
                "20"}},
          }}},
        {ot::blockchain::Type::Litecoin_testnet4,
         {"01000000000000000000000000000000000000000000000000000000000000000000"
          "0000d9ced4ed1130f7b7faad9be25323ffafa33232a17c3edf6cfd97bee6bafbdd97"
          "f60ba158f0ff0f1ee179040001010000000100000000000000000000000000000000"
          "00000000000000000000000000000000ffffffff4804ffff001d0104404e59205469"
          "6d65732030352f4f63742f32303131205374657665204a6f62732c204170706c65e2"
          "80997320566973696f6e6172792c2044696573206174203536ffffffff0100f2052a"
          "010000004341040184710fa689ad5023690c80f3a49c8f13f8d45b8c857fbcbc8bc4"
          "a8e4d3eb4b10f4d4604fa08dce601aaf0f470216fe1b51850b4acf21b179c45070ac"
          "7b03a9ac00000000",
          {
              {ot::blockchain::filter::Type::Basic_BIP158,
               {"014c8c60",
                "02d023da9d271b849f717089aad7e03a515dac982c9fb2cfd952e2ce1c6187"
                "92"}},
              {ot::blockchain::filter::Type::ES,
               {"048b3d60cc5692c061eb30ca191005f1c0",
                "ad242bb97aaf6a8f973dc2054d5356a4fcc87f575b29bbb3e0d953cfaedff8"
                "c6"}},
          }}},
        {ot::blockchain::Type::PKT,
         {"00000000000000000000000000000000000000000000000000000000000000000000"
          "0000df345ba23b13467eec222a919d449dab6506abc555ef307794ecd3d36ac891fb"
          "00000000ffff0f1f0000000001fd04160000000000000000df345ba23b13467eec22"
          "2a919d449dab6506abc555ef307794ecd3d36ac891fb00096e88ffff0f1f03000000"
          "00000000347607000098038000000000ffff0f200000000000000000000000000000"
          "00000000000000000000000000000000000000000000000000000000000000000000"
          "00000000000000000000000000000000000000000000000000000000e79d06f72d77"
          "8459a76a989dbdded6d45b5e4358943c9aab1eb4e42a9c67f9ac317b762fe60198c3"
          "861255552928a179a5e9a6b9b7b7f4b44e02fc3519f92964fbbfb576d1e9ff3c588c"
          "60fb2643602ae1f5695f89460608d3250e57a7755385aaa0de52409159387de4145d"
          "92533cd5f2a0d6d2a21b653311a40bd2556493171cf1beedf894a090626577d8042e"
          "72f9cdab8ab212b2d6ee5ca7b22169a01bf903ab05b248fb8ed5de5a2bb0cd3901fc"
          "2e3270ffa524ed3adfc9d7fe109d0e2755f016386a09eda81bd9707bf681d75cef82"
          "9f3f8ee0903bfdb2c86ff44628df573143ec832f41ae17e575e31848d9cf430930d8"
          "1f41b0d803251b81f8181e543cb25c7dca4f2454f8f8bb86987db019ceffe7f0a2be"
          "807767f85dc903d3b843af448d14d5214b6ad5812b4d82b8cbea25c69c6b87d667f9"
          "c18c2993d500ed902d4c539a7d06ab0ca95afd946fd3702554e4bf9f76a1f087dccf"
          "33356b7efa9149fa6b4949159d03cb6e7d13efe91134a9ed8adc7c7325d39201cb2c"
          "2c1e2191c5e9d3d71dc5d1232e4cfc603fc5aa994d7bb8d190ca3d7c0e2fb9abb68d"
          "f80c2cdfd8d119aec1a9c62c0ef7af9375e56c0330263332c4c879bcda52de73fea2"
          "6781eb3dfa19dd2399b605050198fca80467bdca4a50980a3a37aa552f65caf9634b"
          "18fca475551d0a37dceab5f78c1cfdb48917122137cb74e236800c0684936b9cc0ca"
          "563025cb68609be37869fa8e95bb6fdcd15320b3d5b2fabe9524f464dbfabe36ef95"
          "8170b5d7f25c40938bd287a5540b00e06ccb40f558958b72541e8ca4f4f965e4f788"
          "98085b34fdb6e33b1f588b6d0abc4cb119a8f54e0d949a08afb87979d4c69165ac6b"
          "d9e694369a3903ec24c1e3a52c401c88e035a9f6aed6909f3a2b6dbe60e6fa842400"
          "c4164c21dc4c8b2325b70ad1829bed742717776ff28457b384f4bdd0bf48b2db2d18"
          "f89af671c58ecded320cf289b8fa9cfd53fcd7352de1cff3c41d2f7f8ec6f280d8a9"
          "d6933da42b66a6a3d30d46742e9cd793388a07e5e15b9b220b4209415537214447d3"
          "86abce2c74a24b7dc60ff9ce04a7cad19ab679d0234ac95e535abd57d3ac91747b2f"
          "2cfe1f01bb944502b827fc8d2c5e8f920fb1451880271991e5f5db796ea8d392138c"
          "d18f602dc6deb3149c44e5085fbd77dc997571e4652009b555253eefd215fb009b14"
          "e0e880f67d45e85a8252e457ddd0ace7cfdd5eec6cee070125b50307b7ab0f3983f3"
          "2f58b75fb02133f3e0778c089484d07058e76025855909ff64b7c2ace114b6c302a0"
          "87acc140be90679fe1d0a75300573dc000000000ffff0f2000000000000000000000"
          "00000000000000000000000000000000000000000000000000000000000000000000"
          "00000000000000000000000000000000000000000000000000000000000000007594"
          "c5be146f727d7fb815193044fb2596ceca3a9b62252e5259ed56b7fb63cd2fe906fa"
          "c0f3ff25658998198d9431a48a0be55a0a84333fbdabab0c318930b97d3bb1fa8a8d"
          "deb1587f97c531f81963c70784089465e2ef4f465b8d6bb9bbb27f36971c87b98cca"
          "e3f8d445181b03c97a84ac8a12241b47d9845f966cedade1c31faa857bf2cafae9c7"
          "1041dd23124d4cd4d6dff24cf632e94dd68831639b0f3aba27219cd8a86993660576"
          "0ae408cadeef02c410fc2eeb412bdd7e411614e7830f54ebe0ea6eadae5fe226a67c"
          "0b310d4d4b5d10b47dfe2f165191e69c96e617ef8c3cf763fa49662deb82a2270b49"
          "816f11d56a3493c5e74b0eafbd9492e5fbaa0e0d6600c179a75c2c134e1d6a1c3721"
          "616b6241273b904aec0ef516c402649d032d5e4de8a1fb15bbeb250f5b6993b6bf5a"
          "39314e626d177578fedcc3f7935307321f8f25ae008855b1f19ddf26bcfa1636b3db"
          "132a737b4e1ec50ac9b223670f04a746be5c06e1de90115385c706af7eb947b9b712"
          "f9c14998d31b977ace19a1f2051799fe7aa47bc02f358f2d839891854825a7e7491e"
          "343eb5aa2d468e787989abf9961e21956c5ced5c6a5375e809ad958235fc91989fa4"
          "141230c42ccbf6a50c6ca61e3780d65dbfc112a104cc1da5b1dd7ea024d2e37db0bb"
          "10ab6f06242589cb5383927ac5d130b189d32e4731ec1e8b675caf6c4da531db3c59"
          "8c5da69aa8ddcecae67cefd633fd80f994cec4ad28c2f1421b316999c1043c749b14"
          "a645f785dd56e8fdbc959ff03648336b0c9c9ca3c86bb96738750b855dffa0b74c9c"
          "492580dcbbb892b91d76359aedc0a3d89a447b23f5449433bb7c4554eb6f0eb8ee63"
          "b9df12287f92eb23b3956d3933eeccf88ca9d9fe19a9a29a2821909f3a2b6dbe60e6"
          "fa842400c4164c21dc4c8b2325b70ad1829bed742717776ff28457b384f4bdd0bf48"
          "b2db2d18f89af671c58ecded320cf289b8fa9cfd53fcd7352de1cff3c41d2f7f8ec6"
          "f280d8a9d6933da42b66a6a3d30d46742e9cd793388a07e5e15b9b220b4209415537"
          "214447d386abce2c74a24b7dc60ff9ce04a7cad19ab679d0234ac95e535abd57d3ac"
          "91747b2f2cfe1f01bb944502b827fc8d2c5e8f920fb1451880271991e5f5db796ea8"
          "d392138cd18f602dc6deb3149c44e5085fbd77dc997571e4652009b555253eefd215"
          "fb009b14e0e880f67d45e85a8252e457ddd0ace7cfdd5eec6cee070125b50307b7ab"
          "0f3983f32f58b75fb021ace16c1a11a478a77f48ec8beda4f4912aa3337010343c14"
          "412cbc2f6d8ceb38dc88989cfee876ab00042a8000000000ffff0f20000000000000"
          "00000000000000000000000000000000000000000000000000000000000000000000"
          "00000000000000000000000000000000000000000000000000000000000000000000"
          "00009653aa497eb0bf1f7b9170967201419b6ced537def4363a0b2869d974a91d445"
          "8b4099f8d9a5f8555219c9b6efd193e1c745636d42cd705557c48e47598648c42e1c"
          "94318744855d037b3de60b626de12f06be4ec366527100b35ea8d4626eac5c2461d7"
          "33c072811aa87bb5a39edf46d13a318f948367fe7a130359cd2a1ed04a60ee497723"
          "623b258cecd2581a4d7cc3d7e9d05ae4d63ffcecdd16a19decb7dcffc9a9faccb208"
          "4177e736170f191b99446049304f95a2dad137670c0944a41dd36cd356ad70f65eab"
          "a46732e7976b4d252980db9e82ff554a599aae46dd27886e61a22adf51dbf26be34b"
          "bc766510ddebb15a9bef63ba3052fe7f71252807582e08fa1301fd78138917fec593"
          "f50758f103966bcf45c32071a279367c90d2728d9d13a90c3ee64682b86b80738f4a"
          "d1cc94e8d2c98d70bc99e72b45a68f4719465bd291177ef8675eb9ab2cca7599bb84"
          "70180137e6d0e92dcd13fd60dfa8569175055e76d0df50c79447df8a0d6c64d1d240"
          "aae79168de62becc24097a5da77de3d860efbf3fbb7a737275944899df27a45b9a72"
          "03d813dad5c6ebd0986535a260589a51843ae43bf17902282439ce50ae75ab4ad8f9"
          "94530750fc1b30d7dc364828b76275e3536950834c0afeb17ad04a0a3090cd4e1165"
          "b65727b08c939e355a5c992d87bd80c3a41465bf1b41d304646fbbfb6b3502082829"
          "45b68d3a0440bb8d2dabf1b3767ccc02174499f4084be56f7733052ac65bec5401b9"
          "e627bb4094c8c5fad47a0afb5ab1a7db4de6e318f535013c8db58d16e5455fb0d2aa"
          "32a4d8e4d403412db7ecc718e459e81f09fde3523436ef6104f96201f1fa8c425103"
          "3198d39d0c5a87eae9b9499eb2b3551d4e579103de55354c95b4c3b0cee177cb443e"
          "85e4936100efb659bb7356a52f5d51682673e9cf655c9cec51d100979ffbf74922df"
          "eaecf1bf1ac55933c73d5f3fe927674fd5afc5d5a85e5b8d9779d7352de1cff3c41d"
          "2f7f8ec6f280d8a9d6933da42b66a6a3d30d46742e9cd793388a07e5e15b9b220b42"
          "09415537214447d386abce2c74a24b7dc60ff9ce04a7cad19ab679d0234ac95e535a"
          "bd57d3ac91747b2f2cfe1f01bb944502b827fc8d2c5e8f920fb1451880271991e5f5"
          "db796ea8d392138cd18f602dc6deb3149c44e5085fbd77dc997571e4652009b55525"
          "3eefd215fb009b14e0e880f67d45e85a8252e457ddd0ace7cfdd5eec6cee070125b5"
          "0307b7ab0f3983f32f58b75fb0213ab54f4815c5fb0803d5ddd6d4278fc7105e5a15"
          "aff36d31ba05dd094c5d2b1f59974dd4d04c369300cb318000000000ffff0f200000"
          "00000000000000000000000000000000000000000000000000000000000000000000"
          "00000000000000000000000000000000000000000000000000000000000000000000"
          "000000000000d120d39a00a6aeb9703eaa6410db4990a504e21cdc0ccc4f913441b6"
          "47104b4f0b8b87661db287ccaa443f2920759e0b9524babb4e227c7cc6a0ee765ff2"
          "6b15ac81d3e764d6e4f8527edf236288ca56196d55a51a8c2a7cb9f9fd7f235a459f"
          "b9f77454c0a0cfbd71605850dcb3ad5428614ef576b3cc358a2286bd7089a0459aea"
          "9c86741eb0e4e295ec976b94efcb4441e998c8e51758de78301ed490f799867355ec"
          "d7c57c1d6adfcb2f789f53f47ddd22fb6dad62b4d1b7315001c5b341a265587a3826"
          "5e0e3ea811e53fbee01786efedc6bab28d0ece33016c96a7a52cc1c77cb8eb932020"
          "b883222dbb8a3c9209b7a8e9ef54828b205a63ce185fa813409d4589c203b782fae0"
          "87f59141aca33b8a89af33314de4b215fb61821c03d76f0ac07d2d97e5cad8fe5864"
          "de4269ddb23e0cbf4b53170a4b43da80e7d128f07a471f4ed7e81a9d4ab038cd4cb5"
          "70c810bd4386b882b29d965824d651fdade58fa18a231a2ad288ed5fb0a1716c45c2"
          "4b80a332d5d8cd56d6f663b5b5bec1854bb2477b43bfa482d32577ebe6f775f1349c"
          "71fb98c49eccd2a6a984b29da8664e0715ce25b520e58622a207fd6f58b95a37b095"
          "308e25672bca89d742faebbf8e397d5847a50266d4c8f76bdb9306d105a8a7d83d20"
          "ab07a8769fc1c64ae92233115a91352458a11f329b2b227b07e7aac5439354fd30e4"
          "c1ef22ed6061bdd65020347eb495e40f7ed2d5e5dd6e6cbd34dcdb1078f771c3c93c"
          "8e2f989fd4af8e4704acdae9f0a71e154bf6d0ada9efd1fc6a176299a3ef71fa6504"
          "84d1d7062835a92def53df596633bf39bf0383f30674ea81003187222c48d8d91989"
          "bfd41d40edde7b07c29f8da3e0446cc6f5c58f2941af4418658bc55c20dec60859c8"
          "e8f8545263179afdf5c1b48aedc0fb4b71bf00cd0e53e86d3af5350ba6ed0b283e2f"
          "bbe3333a2856b81f4db572f5193ef5c7561dd6c22e3c0b411fd711529e69bf05811b"
          "2e8ed4fcec0080b506394154245190535ebdf909fbaae9ced09b8f63f925e9170701"
          "598f9757e4db71546f4a4bbe4ad32be2f551f3841e3125881a4750ad6684076e0cf8"
          "a9565c3dfe5140b7b40f3578867a19cf652bef184f9ed2ad63bfa62e16bd8bb52232"
          "d76b171559acaa7c51d56103a83735f0d5b1ae3bc720e5085fbd77dc997571e46520"
          "09b555253eefd215fb009b14e0e880f67d45e85a8252e457ddd0ace7cfdd5eec6cee"
          "070125b50307b7ab0f3983f32f58b75fb0218228bfd8f3d022cd5a99786769f3a3e0"
          "38e68fc7021fd54e8745ea09380d112f5846acb6b0b693a1ad015ae6d04e43116192"
          "dc9edcdcdf52b2ece486afccac3a84da182bc48b69b3dec842c1d5f76abe2f9155a3"
          "22a03808f708af8b589bdd206c338a2fefa693bc9dc232bdb3c03d1fa32b1da8a451"
          "4de4fccb2df8c0ffa2036dc15a92cd13bcb938f3d76853db406ece5f3bbfc6adb556"
          "855af805acdf2b1784fba6e61c1288024f8609b9cee016f3b09c07b1e3257c03fc6f"
          "6a2bf40fd597d326d3eb2bb10c6a4412cb8e260153008a482f7315f2235a3ae044df"
          "7004944fddec3a3eba0095fcb7432c07752f662e57559217925a030083452f8322f7"
          "1a201497ceb1aa8efea84504687932b1630f8440cd8b5b835424a99a6ba6ef531f00"
          "39c96dc9df6ddb1da17db6192d68265aa69fe8e7591d29f883799f4e8530085220cf"
          "e3d522c74c00ec447082de3f07f03e4cf6f427b0f2e54fa73d0ee631d7e632101d48"
          "7173ab63a5a014250a34f900730eb4554c4fcaff9e11e9051a3d7142d74708aadc2e"
          "29e3dec6fa67563527027c92a77e85f39702b90f869548e8d203f4b9166fd7ea1032"
          "e793228ea8ed223fa6d69ffef6c9ceca87df21a33bf16d0095ccd7de5c20364a71f6"
          "3933bc5e9f3269497e6bdc1969d6f4e2106a5ed1adcd971f9af95e595d00953c1527"
          "674ba6b82b0f8f6ce97ded33774c8defd97c5ff1efc54617984d68bde405e946062e"
          "16004f841e6d1cb21d25f844c947d9db391b6394537f0ee65b2670abcb51acb86515"
          "aa98155916420e00dadfa924a79604be0074b78bdba7439f6ac8a0b028c43947f32c"
          "f1bde6af3dc9ffc3b36837c2e20083968aa01025b298c3f70f00028c0ed271ba1f8a"
          "425d46a81e480ad932dce9f46a84d6ccfe205403ad32dc1b571683788d29b2db5a79"
          "3410d9a5843fb29d60ab294e0ccc2f35bfe1593e112a44dd3408760054899838af83"
          "022b08c6b224b92da9961cf8e5c518c082f07b037c87f56d1c711e4564c8c3061b57"
          "767b6ffd2cb2f782d8a02db34ba0d94f6a0f8664af79fff0eac78b47b753df86cdb0"
          "6ebe88017a391df9656bf69eac1536d4237d19b601b632f65c35b264d0b634d17e2d"
          "8882af7cf5859b752801210e474f50eb15a8e67cb2be55332de8c389d1beeddfc275"
          "a3efeeb25ef6eadc57f4ab65436f7600d93cd72a0ee92af81941141ba58b6e361510"
          "f10bf66ff61ca2a3b6e0c83114d96bf382431fa21c00c9d818dc76721ed0ed098385"
          "60630ce2e2fc3ff2796727f0ded2147f68c040bf0b06c99184f0b53b13e966dd46b6"
          "224663f591dcb06be2c15398ad79af6155478d888c0cec4d0f008f0469a084a21a00"
          "6ad610832938232cd672079fd672c29cfe44a9fe28029e4474b1d0efdf09ca6c9995"
          "8969864e1a0483236c9a496f6753bd1dae2169f4a4a665d28907e5347aa30b181fa8"
          "91a3d13c97612292424a7d21f89806e9ae3161be2e1067f7e5821c352cf985af08d9"
          "90b2d5595dcf6aee29ba8f6a906990bb2407447e64dc31fdbb925dba728427683ef1"
          "6e6fcde7b982390314a10cc5bd8c3a3fc9d4b1544a966301dbfda478712ea9de748e"
          "d1120bd864dab49694680dfdf647cb5d263d0a591c737fd3815475cbf0006bf0b638"
          "870865f9118936e144b4e7315763a5e526450325e1966ed32af3ec4f5c07231e161f"
          "4f006d0b61cd3a747951d29a6af505a27264206786b8de5339ea1972c7e11027e77f"
          "90a5c9b11f5d2800490da63f1a94ffbb0bccc057f1be13eeae5cc8da783d3b84e2ae"
          "3aa424f54a663a4a9f9e67810f00b833ec0156377a6b96eb8b53e335f018af4b8be9"
          "4118485b2d3b53652e890526d1a41bded7141400a8cc33116507392c3db3dddf3291"
          "d97543c77e9a2c616dfe130f23d0bc3733b0f2843d32c51d0e04e7932ad21ec5e9be"
          "6dd6b86e541e2323ccf8b209ad0940b7222d4aaa91d8837fe42cf46b785af711ea8c"
          "6600320be68fcd657241e8efb16dde17e25f5adcf601aed934acfb3a82a2245a46f8"
          "b224527eb3ca48beab1f052a044b9a7ef7d12a11c7e81bc72b0d3fce26f522a6180a"
          "762742d1e0ea79950a000f653cf348876d1b2a42b4c7524dc906089023d96eff593c"
          "6eb9f0f4ecbd32480000010100000001000000000000000000000000000000000000"
          "0000000000000000000000000000ffffffff0100ffffffff02000000801104000022"
          "0020d5c1005c0d4012d3ae2672319e7f9eb15a57516aeefabbbc062265f67e308f2b"
          "0000000000000000326a3009f91102ffff0f20f935b3001ef51ba8f24921a404bc37"
          "6a0c713274bd1cc68c2c57f66f5c0be7ca001000000000000000000000",
          {
              {ot::blockchain::filter::Type::Basic_BIP158,
               {"01902168",
                "526b0656def40fcb65ef87a75337001fae57a1d17dc17e103fb536cfddedd3"
                "6c"}},
              {ot::blockchain::filter::Type::ES,
               {"02649a42b26e818d40",
                "155e1700eff3f9019ba1716316295a8753ec44d2a7730eee1c1c73e2b511e1"
                "34"}},
          }}},
    };
}  // namespace ottest
