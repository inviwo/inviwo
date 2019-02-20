/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/

/*********************************************************************************
 * // https://github.com/LordJZ/consthash
 *
 * The MIT License(MIT)
 *
 * Copyright(c) 2015
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files(the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions :

 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *********************************************************************************/

#ifndef IVW_CONSTEXPRHASH_H
#define IVW_CONSTEXPRHASH_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>

namespace inviwo {

namespace util {

namespace detail {

static uint64_t constexpr crc_table[] = {
    0x0000000000000000ULL, 0x24854997ba2f81e7ULL, 0x490a932f745f03ceULL, 0x6d8fdab8ce708229ULL,
    0x9215265ee8be079cULL, 0xb6906fc95291867bULL, 0xdb1fb5719ce10452ULL, 0xff9afce626ce85b5ULL,
    0x66daad56789639abULL, 0x425fe4c1c2b9b84cULL, 0x2fd03e790cc93a65ULL, 0x0b5577eeb6e6bb82ULL,
    0xf4cf8b0890283e37ULL, 0xd04ac29f2a07bfd0ULL, 0xbdc51827e4773df9ULL, 0x994051b05e58bc1eULL,
    0xcdb55aacf12c7356ULL, 0xe930133b4b03f2b1ULL, 0x84bfc98385737098ULL, 0xa03a80143f5cf17fULL,
    0x5fa07cf2199274caULL, 0x7b253565a3bdf52dULL, 0x16aaefdd6dcd7704ULL, 0x322fa64ad7e2f6e3ULL,
    0xab6ff7fa89ba4afdULL, 0x8feabe6d3395cb1aULL, 0xe26564d5fde54933ULL, 0xc6e02d4247cac8d4ULL,
    0x397ad1a461044d61ULL, 0x1dff9833db2bcc86ULL, 0x7070428b155b4eafULL, 0x54f50b1caf74cf48ULL,
    0xd99a54b24bb2d03fULL, 0xfd1f1d25f19d51d8ULL, 0x9090c79d3fedd3f1ULL, 0xb4158e0a85c25216ULL,
    0x4b8f72eca30cd7a3ULL, 0x6f0a3b7b19235644ULL, 0x0285e1c3d753d46dULL, 0x2600a8546d7c558aULL,
    0xbf40f9e43324e994ULL, 0x9bc5b073890b6873ULL, 0xf64a6acb477bea5aULL, 0xd2cf235cfd546bbdULL,
    0x2d55dfbadb9aee08ULL, 0x09d0962d61b56fefULL, 0x645f4c95afc5edc6ULL, 0x40da050215ea6c21ULL,
    0x142f0e1eba9ea369ULL, 0x30aa478900b1228eULL, 0x5d259d31cec1a0a7ULL, 0x79a0d4a674ee2140ULL,
    0x863a28405220a4f5ULL, 0xa2bf61d7e80f2512ULL, 0xcf30bb6f267fa73bULL, 0xebb5f2f89c5026dcULL,
    0x72f5a348c2089ac2ULL, 0x5670eadf78271b25ULL, 0x3bff3067b657990cULL, 0x1f7a79f00c7818ebULL,
    0xe0e085162ab69d5eULL, 0xc465cc8190991cb9ULL, 0xa9ea16395ee99e90ULL, 0x8d6f5faee4c61f77ULL,
    0xf1c4488f3e8f96edULL, 0xd541011884a0170aULL, 0xb8cedba04ad09523ULL, 0x9c4b9237f0ff14c4ULL,
    0x63d16ed1d6319171ULL, 0x475427466c1e1096ULL, 0x2adbfdfea26e92bfULL, 0x0e5eb46918411358ULL,
    0x971ee5d94619af46ULL, 0xb39bac4efc362ea1ULL, 0xde1476f63246ac88ULL, 0xfa913f6188692d6fULL,
    0x050bc387aea7a8daULL, 0x218e8a101488293dULL, 0x4c0150a8daf8ab14ULL, 0x6884193f60d72af3ULL,
    0x3c711223cfa3e5bbULL, 0x18f45bb4758c645cULL, 0x757b810cbbfce675ULL, 0x51fec89b01d36792ULL,
    0xae64347d271de227ULL, 0x8ae17dea9d3263c0ULL, 0xe76ea7525342e1e9ULL, 0xc3ebeec5e96d600eULL,
    0x5aabbf75b735dc10ULL, 0x7e2ef6e20d1a5df7ULL, 0x13a12c5ac36adfdeULL, 0x372465cd79455e39ULL,
    0xc8be992b5f8bdb8cULL, 0xec3bd0bce5a45a6bULL, 0x81b40a042bd4d842ULL, 0xa531439391fb59a5ULL,
    0x285e1c3d753d46d2ULL, 0x0cdb55aacf12c735ULL, 0x61548f120162451cULL, 0x45d1c685bb4dc4fbULL,
    0xba4b3a639d83414eULL, 0x9ece73f427acc0a9ULL, 0xf341a94ce9dc4280ULL, 0xd7c4e0db53f3c367ULL,
    0x4e84b16b0dab7f79ULL, 0x6a01f8fcb784fe9eULL, 0x078e224479f47cb7ULL, 0x230b6bd3c3dbfd50ULL,
    0xdc919735e51578e5ULL, 0xf814dea25f3af902ULL, 0x959b041a914a7b2bULL, 0xb11e4d8d2b65faccULL,
    0xe5eb469184113584ULL, 0xc16e0f063e3eb463ULL, 0xace1d5bef04e364aULL, 0x88649c294a61b7adULL,
    0x77fe60cf6caf3218ULL, 0x537b2958d680b3ffULL, 0x3ef4f3e018f031d6ULL, 0x1a71ba77a2dfb031ULL,
    0x8331ebc7fc870c2fULL, 0xa7b4a25046a88dc8ULL, 0xca3b78e888d80fe1ULL, 0xeebe317f32f78e06ULL,
    0x1124cd9914390bb3ULL, 0x35a1840eae168a54ULL, 0x582e5eb66066087dULL, 0x7cab1721da49899aULL,
    0xa17870f5d4f51b49ULL, 0x85fd39626eda9aaeULL, 0xe872e3daa0aa1887ULL, 0xccf7aa4d1a859960ULL,
    0x336d56ab3c4b1cd5ULL, 0x17e81f3c86649d32ULL, 0x7a67c58448141f1bULL, 0x5ee28c13f23b9efcULL,
    0xc7a2dda3ac6322e2ULL, 0xe3279434164ca305ULL, 0x8ea84e8cd83c212cULL, 0xaa2d071b6213a0cbULL,
    0x55b7fbfd44dd257eULL, 0x7132b26afef2a499ULL, 0x1cbd68d2308226b0ULL, 0x383821458aada757ULL,
    0x6ccd2a5925d9681fULL, 0x484863ce9ff6e9f8ULL, 0x25c7b97651866bd1ULL, 0x0142f0e1eba9ea36ULL,
    0xfed80c07cd676f83ULL, 0xda5d45907748ee64ULL, 0xb7d29f28b9386c4dULL, 0x9357d6bf0317edaaULL,
    0x0a17870f5d4f51b4ULL, 0x2e92ce98e760d053ULL, 0x431d14202910527aULL, 0x67985db7933fd39dULL,
    0x9802a151b5f15628ULL, 0xbc87e8c60fded7cfULL, 0xd108327ec1ae55e6ULL, 0xf58d7be97b81d401ULL,
    0x78e224479f47cb76ULL, 0x5c676dd025684a91ULL, 0x31e8b768eb18c8b8ULL, 0x156dfeff5137495fULL,
    0xeaf7021977f9cceaULL, 0xce724b8ecdd64d0dULL, 0xa3fd913603a6cf24ULL, 0x8778d8a1b9894ec3ULL,
    0x1e388911e7d1f2ddULL, 0x3abdc0865dfe733aULL, 0x57321a3e938ef113ULL, 0x73b753a929a170f4ULL,
    0x8c2daf4f0f6ff541ULL, 0xa8a8e6d8b54074a6ULL, 0xc5273c607b30f68fULL, 0xe1a275f7c11f7768ULL,
    0xb5577eeb6e6bb820ULL, 0x91d2377cd44439c7ULL, 0xfc5dedc41a34bbeeULL, 0xd8d8a453a01b3a09ULL,
    0x274258b586d5bfbcULL, 0x03c711223cfa3e5bULL, 0x6e48cb9af28abc72ULL, 0x4acd820d48a53d95ULL,
    0xd38dd3bd16fd818bULL, 0xf7089a2aacd2006cULL, 0x9a87409262a28245ULL, 0xbe020905d88d03a2ULL,
    0x4198f5e3fe438617ULL, 0x651dbc74446c07f0ULL, 0x089266cc8a1c85d9ULL, 0x2c172f5b3033043eULL,
    0x50bc387aea7a8da4ULL, 0x743971ed50550c43ULL, 0x19b6ab559e258e6aULL, 0x3d33e2c2240a0f8dULL,
    0xc2a91e2402c48a38ULL, 0xe62c57b3b8eb0bdfULL, 0x8ba38d0b769b89f6ULL, 0xaf26c49cccb40811ULL,
    0x3666952c92ecb40fULL, 0x12e3dcbb28c335e8ULL, 0x7f6c0603e6b3b7c1ULL, 0x5be94f945c9c3626ULL,
    0xa473b3727a52b393ULL, 0x80f6fae5c07d3274ULL, 0xed79205d0e0db05dULL, 0xc9fc69cab42231baULL,
    0x9d0962d61b56fef2ULL, 0xb98c2b41a1797f15ULL, 0xd403f1f96f09fd3cULL, 0xf086b86ed5267cdbULL,
    0x0f1c4488f3e8f96eULL, 0x2b990d1f49c77889ULL, 0x4616d7a787b7faa0ULL, 0x62939e303d987b47ULL,
    0xfbd3cf8063c0c759ULL, 0xdf568617d9ef46beULL, 0xb2d95caf179fc497ULL, 0x965c1538adb04570ULL,
    0x69c6e9de8b7ec0c5ULL, 0x4d43a04931514122ULL, 0x20cc7af1ff21c30bULL, 0x04493366450e42ecULL,
    0x89266cc8a1c85d9bULL, 0xada3255f1be7dc7cULL, 0xc02cffe7d5975e55ULL, 0xe4a9b6706fb8dfb2ULL,
    0x1b334a9649765a07ULL, 0x3fb60301f359dbe0ULL, 0x5239d9b93d2959c9ULL, 0x76bc902e8706d82eULL,
    0xeffcc19ed95e6430ULL, 0xcb7988096371e5d7ULL, 0xa6f652b1ad0167feULL, 0x82731b26172ee619ULL,
    0x7de9e7c031e063acULL, 0x596cae578bcfe24bULL, 0x34e374ef45bf6062ULL, 0x10663d78ff90e185ULL,
    0x4493366450e42ecdULL, 0x60167ff3eacbaf2aULL, 0x0d99a54b24bb2d03ULL, 0x291cecdc9e94ace4ULL,
    0xd686103ab85a2951ULL, 0xf20359ad0275a8b6ULL, 0x9f8c8315cc052a9fULL, 0xbb09ca82762aab78ULL,
    0x22499b3228721766ULL, 0x06ccd2a5925d9681ULL, 0x6b43081d5c2d14a8ULL, 0x4fc6418ae602954fULL,
    0xb05cbd6cc0cc10faULL, 0x94d9f4fb7ae3911dULL, 0xf9562e43b4931334ULL, 0xddd367d40ebc92d3ULL};

constexpr uint64_t crc64impl(uint64_t prevCrc, const char* str, size_t size) {
    return !size
               ? prevCrc
               : crc64impl((prevCrc >> 8) ^ crc_table[(prevCrc ^ *str) & 0xff], str + 1, size - 1);
}

constexpr uint64_t crc64(const char* str, size_t size) {
    return crc64impl(0xffffffff, str, size) ^ 0xffffffff;
}

}  // namespace detail

template <size_t len>
constexpr uint64_t constexpr_hash(const char (&str)[len]) {
    return detail::crc64(str, len);
}

}  // namespace util

}  // namespace inviwo

#endif  // IVW_CONSTEXPRHASH_H
