/*
 * Copyright 2017 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SYSTEM_SECURITY_KEYSTORE_KEYMASTER_TAGS_H_
#define SYSTEM_SECURITY_KEYSTORE_KEYMASTER_TAGS_H_

/**
 * This header contains various definitions that make working with keymaster tags safer and easier.
 *
 * It makes use of a fair amount of template metaprogramming. The metaprogramming serves the purpose
 * of making it impossible to make certain classes of mistakes when operating on keymaster
 * authorizations.  For example, it's an error to create a KeyParameter with tag == Tag::PURPOSE
 * and then to assign Algorithm::RSA to algorithm element of its union. But because the user
 * must choose the union field, there could be a mismatch which the compiler has now way to
 * diagnose.
 *
 * The machinery in this header solves these problems by describing which union field corresponds
 * to which Tag. Central to this mechanism is the template TypedTag. It has zero size and binds a
 * numeric Tag to a type that the compiler understands. By means of the macro DECLARE_TYPED_TAG,
 * we declare types for each of the tags defined in hardware/interfaces/keymaster/4.0/types.hal.
 *
 * The macro DECLARE_TYPED_TAG(name) generates a typename TAG_name_t and a zero sized instance
 * TAG_name. Once these typed tags have been declared we define metafunctions mapping the each tag
 * to its value c++ type and the correct union element of KeyParameter. This is done by means of
 * the macros MAKE_TAG_*VALUE_ACCESSOR, which generates TypedTag2ValueType, a metafunction mapping
 * a typed tag to the corresponding c++ type, and access function, accessTagValue returning a
 * reference to the correct element of KeyParameter.
 * E.g.:
 *      given "KeyParameter param;" then "accessTagValue(TAG_PURPOSE, param)"
 *      yields a reference to param.f.purpose
 * If used in an assignment the compiler can now check the compatibility of the assigned value.
 *
 * For convenience we also provide the constructor like function Authorization().
 * Authorization takes a typed tag and a value and checks at compile time whether the value given
 * is suitable for the given tag. At runtime it creates a new KeyParameter initialized with the
 * given tag and value and returns it by value.
 *
 * The second convenience function, authorizationValue, allows access to the KeyParameter value in
 * a safe way. It takes a typed tag and a KeyParameter and returns a reference to the value wrapped
 * by NullOr. NullOr has out-of-band information about whether it is save to access the wrapped
 * reference.
 * E.g.:
 *      auto param = Authorization(TAG_ALGORITM, Algorithm::RSA);
 *      auto value1 = authorizationValue(TAG_PURPOSE, param);
 *      auto value2 = authorizationValue(TAG_ALGORITM, param);
 * value1.isOk() yields false, but value2.isOk() yields true, thus value2.value() is save to access.
 */

#include <android/hardware/keymaster/4.0/IKeymasterDevice.h>

#include <type_traits>

namespace android {
namespace hardware {
namespace keymaster {
namespace V4_0 {

// The following create the numeric values that KM_TAG_PADDING and KM_TAG_DIGEST used to have.  We
// need these old values to be able to support old keys that use them.
static const int32_t KM_TAG_DIGEST_OLD = static_cast<int32_t>(TagType::ENUM) | 5;
static const int32_t KM_TAG_PADDING_OLD = static_cast<int32_t>(TagType::ENUM) | 7;

static const int32_t KM_TAG_FBE_ICE = static_cast<int32_t>(TagType::BOOL) | 16201;
static const int32_t KM_TAG_KEY_TYPE = static_cast<int32_t>(TagType::UINT) | 16202;

constexpr TagType typeFromTag(Tag tag) {
    return static_cast<TagType>(static_cast<uint32_t>(tag) & static_cast<uint32_t>(0xf0000000));
}

/**
 * TypedTag is a templatized version of Tag, which provides compile-time checking of
 * keymaster tag types. Instances are convertible to Tag, so they can be used wherever
 * Tag is expected, and because they encode the tag type it's possible to create
 * function overloads that only operate on tags with a particular type.
 */
template <TagType tag_type, Tag tag>
struct TypedTag {
    inline TypedTag() {
        // Ensure that it's impossible to create a TypedTag instance whose 'tag' doesn't have type
        // 'tag_type'.  Attempting to instantiate a tag with the wrong type will result in a compile
        // error (no match for template specialization StaticAssert<false>), with no run-time cost.
        static_assert(typeFromTag(tag) == tag_type, "mismatch between tag and tag_type");
    }
    operator Tag() const { return tag; }
    int32_t maskedTag() { return tag & 0x0FFFFFFF; }
};

template <Tag tag>
struct Tag2TypedTag {
    typedef TypedTag<typeFromTag(tag), tag> type;
};

#define DECLARE_TYPED_TAG(name)                                    \
    typedef typename Tag2TypedTag<Tag::name>::type TAG_##name##_t; \
    static TAG_##name##_t TAG_##name;

DECLARE_TYPED_TAG(ACTIVE_DATETIME);
DECLARE_TYPED_TAG(ALGORITHM);
DECLARE_TYPED_TAG(ALLOW_WHILE_ON_BODY);
DECLARE_TYPED_TAG(APPLICATION_DATA);
DECLARE_TYPED_TAG(APPLICATION_ID);
DECLARE_TYPED_TAG(ASSOCIATED_DATA);
DECLARE_TYPED_TAG(ATTESTATION_APPLICATION_ID);
DECLARE_TYPED_TAG(ATTESTATION_CHALLENGE);
DECLARE_TYPED_TAG(AUTH_TIMEOUT);
DECLARE_TYPED_TAG(BLOB_USAGE_REQUIREMENTS);
DECLARE_TYPED_TAG(BLOCK_MODE);
DECLARE_TYPED_TAG(BOOTLOADER_ONLY);
DECLARE_TYPED_TAG(BOOT_PATCHLEVEL);
DECLARE_TYPED_TAG(CALLER_NONCE);
DECLARE_TYPED_TAG(CONFIRMATION_TOKEN);
DECLARE_TYPED_TAG(CREATION_DATETIME);
DECLARE_TYPED_TAG(DIGEST);
DECLARE_TYPED_TAG(EC_CURVE);
DECLARE_TYPED_TAG(HARDWARE_TYPE);
DECLARE_TYPED_TAG(INCLUDE_UNIQUE_ID);
DECLARE_TYPED_TAG(INVALID);
DECLARE_TYPED_TAG(KEY_SIZE);
DECLARE_TYPED_TAG(MAC_LENGTH);
DECLARE_TYPED_TAG(MAX_USES_PER_BOOT);
DECLARE_TYPED_TAG(MIN_MAC_LENGTH);
DECLARE_TYPED_TAG(MIN_SECONDS_BETWEEN_OPS);
DECLARE_TYPED_TAG(NONCE);
DECLARE_TYPED_TAG(NO_AUTH_REQUIRED);
DECLARE_TYPED_TAG(ORIGIN);
DECLARE_TYPED_TAG(ORIGINATION_EXPIRE_DATETIME);
DECLARE_TYPED_TAG(OS_PATCHLEVEL);
DECLARE_TYPED_TAG(OS_VERSION);
DECLARE_TYPED_TAG(PADDING);
DECLARE_TYPED_TAG(PURPOSE);
DECLARE_TYPED_TAG(RESET_SINCE_ID_ROTATION);
DECLARE_TYPED_TAG(ROLLBACK_RESISTANCE);
DECLARE_TYPED_TAG(ROOT_OF_TRUST);
DECLARE_TYPED_TAG(RSA_PUBLIC_EXPONENT);
DECLARE_TYPED_TAG(TRUSTED_CONFIRMATION_REQUIRED);
DECLARE_TYPED_TAG(TRUSTED_USER_PRESENCE_REQUIRED);
DECLARE_TYPED_TAG(UNIQUE_ID);
DECLARE_TYPED_TAG(UNLOCKED_DEVICE_REQUIRED);
DECLARE_TYPED_TAG(USAGE_EXPIRE_DATETIME);
DECLARE_TYPED_TAG(USER_AUTH_TYPE);
DECLARE_TYPED_TAG(USER_ID);
DECLARE_TYPED_TAG(USER_SECURE_ID);
DECLARE_TYPED_TAG(VENDOR_PATCHLEVEL);

template <typename... Elems>
struct MetaList {};

using all_tags_t =
    MetaList<TAG_INVALID_t, TAG_KEY_SIZE_t, TAG_MAC_LENGTH_t, TAG_CALLER_NONCE_t,
             TAG_MIN_MAC_LENGTH_t, TAG_RSA_PUBLIC_EXPONENT_t, TAG_INCLUDE_UNIQUE_ID_t,
             TAG_ACTIVE_DATETIME_t, TAG_ORIGINATION_EXPIRE_DATETIME_t, TAG_USAGE_EXPIRE_DATETIME_t,
             TAG_MIN_SECONDS_BETWEEN_OPS_t, TAG_MAX_USES_PER_BOOT_t, TAG_USER_ID_t,
             TAG_USER_SECURE_ID_t, TAG_NO_AUTH_REQUIRED_t, TAG_AUTH_TIMEOUT_t,
             TAG_ALLOW_WHILE_ON_BODY_t, TAG_UNLOCKED_DEVICE_REQUIRED_t, TAG_APPLICATION_ID_t,
             TAG_APPLICATION_DATA_t, TAG_CREATION_DATETIME_t, TAG_ROLLBACK_RESISTANCE_t,
             TAG_HARDWARE_TYPE_t, TAG_ROOT_OF_TRUST_t, TAG_ASSOCIATED_DATA_t, TAG_NONCE_t,
             TAG_BOOTLOADER_ONLY_t, TAG_OS_VERSION_t, TAG_OS_PATCHLEVEL_t, TAG_UNIQUE_ID_t,
             TAG_ATTESTATION_CHALLENGE_t, TAG_ATTESTATION_APPLICATION_ID_t,
             TAG_RESET_SINCE_ID_ROTATION_t, TAG_PURPOSE_t, TAG_ALGORITHM_t, TAG_BLOCK_MODE_t,
             TAG_DIGEST_t, TAG_PADDING_t, TAG_BLOB_USAGE_REQUIREMENTS_t, TAG_ORIGIN_t,
             TAG_USER_AUTH_TYPE_t, TAG_EC_CURVE_t, TAG_BOOT_PATCHLEVEL_t, TAG_VENDOR_PATCHLEVEL_t,
             TAG_TRUSTED_CONFIRMATION_REQUIRED_t, TAG_TRUSTED_USER_PRESENCE_REQUIRED_t>;

template <typename TypedTagType>
struct TypedTag2ValueType;

#define MAKE_TAG_VALUE_ACCESSOR(tag_type, field_name)                              \
    template <Tag tag>                                                             \
    struct TypedTag2ValueType<TypedTag<tag_type, tag>> {                           \
        typedef decltype(static_cast<KeyParameter*>(nullptr)->field_name) type;    \
    };                                                                             \
    template <Tag tag>                                                             \
    inline auto accessTagValue(TypedTag<tag_type, tag>, const KeyParameter& param) \
        ->const decltype(param.field_name)& {                                      \
        return param.field_name;                                                   \
    }                                                                              \
    template <Tag tag>                                                             \
    inline auto accessTagValue(TypedTag<tag_type, tag>, KeyParameter& param)       \
        ->decltype(param.field_name)& {                                            \
        return param.field_name;                                                   \
    }

MAKE_TAG_VALUE_ACCESSOR(TagType::ULONG, f.longInteger)
MAKE_TAG_VALUE_ACCESSOR(TagType::ULONG_REP, f.longInteger)
MAKE_TAG_VALUE_ACCESSOR(TagType::DATE, f.dateTime)
MAKE_TAG_VALUE_ACCESSOR(TagType::UINT, f.integer)
MAKE_TAG_VALUE_ACCESSOR(TagType::UINT_REP, f.integer)
MAKE_TAG_VALUE_ACCESSOR(TagType::BOOL, f.boolValue)
MAKE_TAG_VALUE_ACCESSOR(TagType::BYTES, blob)
MAKE_TAG_VALUE_ACCESSOR(TagType::BIGNUM, blob)

#define MAKE_TAG_ENUM_VALUE_ACCESSOR(typed_tag, field_name)                     \
    template <>                                                                 \
    struct TypedTag2ValueType<decltype(typed_tag)> {                            \
        typedef decltype(static_cast<KeyParameter*>(nullptr)->field_name) type; \
    };                                                                          \
    inline auto accessTagValue(decltype(typed_tag), const KeyParameter& param)  \
        ->const decltype(param.field_name)& {                                   \
        return param.field_name;                                                \
    }                                                                           \
    inline auto accessTagValue(decltype(typed_tag), KeyParameter& param)        \
        ->decltype(param.field_name)& {                                         \
        return param.field_name;                                                \
    }

MAKE_TAG_ENUM_VALUE_ACCESSOR(TAG_ALGORITHM, f.algorithm)
MAKE_TAG_ENUM_VALUE_ACCESSOR(TAG_BLOB_USAGE_REQUIREMENTS, f.keyBlobUsageRequirements)
MAKE_TAG_ENUM_VALUE_ACCESSOR(TAG_BLOCK_MODE, f.blockMode)
MAKE_TAG_ENUM_VALUE_ACCESSOR(TAG_DIGEST, f.digest)
MAKE_TAG_ENUM_VALUE_ACCESSOR(TAG_EC_CURVE, f.ecCurve)
MAKE_TAG_ENUM_VALUE_ACCESSOR(TAG_ORIGIN, f.origin)
MAKE_TAG_ENUM_VALUE_ACCESSOR(TAG_PADDING, f.paddingMode)
MAKE_TAG_ENUM_VALUE_ACCESSOR(TAG_PURPOSE, f.purpose)
MAKE_TAG_ENUM_VALUE_ACCESSOR(TAG_USER_AUTH_TYPE, f.hardwareAuthenticatorType)
MAKE_TAG_ENUM_VALUE_ACCESSOR(TAG_HARDWARE_TYPE, f.hardwareType)

template <TagType tag_type, Tag tag, typename ValueT>
inline KeyParameter makeKeyParameter(TypedTag<tag_type, tag> ttag, ValueT&& value) {
    KeyParameter param;
    param.tag = tag;
    param.f.longInteger = 0;
    accessTagValue(ttag, param) = std::forward<ValueT>(value);
    return param;
}

// the boolean case
template <Tag tag>
inline KeyParameter makeKeyParameter(TypedTag<TagType::BOOL, tag>) {
    KeyParameter param;
    param.tag = tag;
    param.f.boolValue = true;
    return param;
}

template <typename... Pack>
struct FirstOrNoneHelper;
template <typename First>
struct FirstOrNoneHelper<First> {
    typedef First type;
};
template <>
struct FirstOrNoneHelper<> {
    struct type {};
};

template <typename... Pack>
using FirstOrNone = typename FirstOrNoneHelper<Pack...>::type;

template <TagType tag_type, Tag tag, typename... Args>
inline KeyParameter Authorization(TypedTag<tag_type, tag> ttag, Args&&... args) {
    static_assert(tag_type != TagType::BOOL || (sizeof...(args) == 0),
                  "TagType::BOOL Authorizations do not take parameters. Presence is truth.");
    static_assert(tag_type == TagType::BOOL || (sizeof...(args) == 1),
                  "Authorization other then TagType::BOOL take exactly one parameter.");
    static_assert(
        tag_type == TagType::BOOL ||
            std::is_convertible<std::remove_cv_t<std::remove_reference_t<FirstOrNone<Args...>>>,
                                typename TypedTag2ValueType<TypedTag<tag_type, tag>>::type>::value,
        "Invalid argument type for given tag.");

    return makeKeyParameter(ttag, std::forward<Args>(args)...);
}

/**
 * This class wraps a (mostly return) value and stores whether or not the wrapped value is valid out
 * of band. Note that if the wrapped value is a reference it is unsafe to access the value if
 * !isOk(). If the wrapped type is a pointer or value and !isOk(), it is still safe to access the
 * wrapped value. In this case the pointer will be NULL though, and the value will be default
 * constructed.
 */
template <typename ValueT>
class NullOr {
    using internal_t = std::conditional_t<std::is_lvalue_reference<ValueT>::value,
                                          std::remove_reference_t<ValueT>*, ValueT>;

    struct pointer_initializer {
        static std::nullptr_t init() { return nullptr; }
    };
    struct value_initializer {
        static ValueT init() { return ValueT(); }
    };
    struct value_pointer_deref_t {
        static ValueT& deref(ValueT& v) { return v; }
    };
    struct reference_deref_t {
        static auto& deref(internal_t v) { return *v; }
    };
    using initializer_t = std::conditional_t<std::is_lvalue_reference<ValueT>::value ||
                                                     std::is_pointer<ValueT>::value,
                                             pointer_initializer, value_initializer>;
    using deref_t = std::conditional_t<std::is_lvalue_reference<ValueT>::value, reference_deref_t,
                                       value_pointer_deref_t>;

  public:
    NullOr() : value_(initializer_t::init()), null_(true) {}
    template <typename T>
    NullOr(T&& value, typename std::enable_if<
                              !std::is_lvalue_reference<ValueT>::value &&
                                      std::is_same<std::decay_t<ValueT>, std::decay_t<T>>::value,
                              int>::type = 0)
        : value_(std::forward<ValueT>(value)), null_(false) {}
    template <typename T>
    NullOr(T& value, typename std::enable_if<
                             std::is_lvalue_reference<ValueT>::value &&
                                     std::is_same<std::decay_t<ValueT>, std::decay_t<T>>::value,
                             int>::type = 0)
        : value_(&value), null_(false) {}

    bool isOk() const { return !null_; }

    const ValueT& value() const& { return deref_t::deref(value_); }
    ValueT& value() & { return deref_t::deref(value_); }
    ValueT&& value() && { return std::move(deref_t::deref(value_)); }

  private:
    internal_t value_;
    bool null_;
};

template <typename T>
std::remove_reference_t<T> NullOrOr(T&& v) {
    if (v.isOk()) return v;
    return {};
}

template <typename Head, typename... Tail>
std::remove_reference_t<Head> NullOrOr(Head&& head, Tail&&... tail) {
    if (head.isOk()) return head;
    return NullOrOr(std::forward<Tail>(tail)...);
}

template <typename Default, typename Wrapped>
std::remove_reference_t<Wrapped> defaultOr(NullOr<Wrapped>&& optional, Default&& def) {
    static_assert(std::is_convertible<std::remove_reference_t<Default>,
                                      std::remove_reference_t<Wrapped>>::value,
                  "Type of default value must match the type wrapped by NullOr");
    if (optional.isOk()) return optional.value();
    return def;
}

template <TagType tag_type, Tag tag>
inline NullOr<const typename TypedTag2ValueType<TypedTag<tag_type, tag>>::type&> authorizationValue(
    TypedTag<tag_type, tag> ttag, const KeyParameter& param) {
    if (tag != param.tag) return {};
    return accessTagValue(ttag, param);
}

inline bool operator==(const KeyParameter& a, const KeyParameter& b) {
    if (a.tag != b.tag) {
        return false;
    }

    switch (a.tag) {
        /* Boolean tags */
        case Tag::INVALID:
        case Tag::CALLER_NONCE:
        case Tag::INCLUDE_UNIQUE_ID:
        case Tag::BOOTLOADER_ONLY:
        case Tag::NO_AUTH_REQUIRED:
        case Tag::ALLOW_WHILE_ON_BODY:
        case Tag::UNLOCKED_DEVICE_REQUIRED:
        case Tag::ROLLBACK_RESISTANCE:
        case Tag::RESET_SINCE_ID_ROTATION:
        case Tag::TRUSTED_CONFIRMATION_REQUIRED:
        case Tag::TRUSTED_USER_PRESENCE_REQUIRED:
            return true;

        /* Integer tags */
        case Tag::KEY_SIZE:
        case Tag::MIN_MAC_LENGTH:
        case Tag::MIN_SECONDS_BETWEEN_OPS:
        case Tag::MAX_USES_PER_BOOT:
        case Tag::OS_VERSION:
        case Tag::OS_PATCHLEVEL:
        case Tag::MAC_LENGTH:
        case Tag::USER_ID:
        case Tag::AUTH_TIMEOUT:
        case Tag::VENDOR_PATCHLEVEL:
        case Tag::BOOT_PATCHLEVEL:
            return a.f.integer == b.f.integer;

        /* Long integer tags */
        case Tag::RSA_PUBLIC_EXPONENT:
        case Tag::USER_SECURE_ID:
            return a.f.longInteger == b.f.longInteger;

        /* Date-time tags */
        case Tag::ACTIVE_DATETIME:
        case Tag::ORIGINATION_EXPIRE_DATETIME:
        case Tag::USAGE_EXPIRE_DATETIME:
        case Tag::CREATION_DATETIME:
            return a.f.dateTime == b.f.dateTime;

        /* Bytes tags */
        case Tag::APPLICATION_ID:
        case Tag::APPLICATION_DATA:
        case Tag::ROOT_OF_TRUST:
        case Tag::UNIQUE_ID:
        case Tag::ATTESTATION_CHALLENGE:
        case Tag::ATTESTATION_APPLICATION_ID:
        case Tag::ATTESTATION_ID_BRAND:
        case Tag::ATTESTATION_ID_DEVICE:
        case Tag::ATTESTATION_ID_PRODUCT:
        case Tag::ATTESTATION_ID_SERIAL:
        case Tag::ATTESTATION_ID_IMEI:
        case Tag::ATTESTATION_ID_MEID:
        case Tag::ATTESTATION_ID_MANUFACTURER:
        case Tag::ATTESTATION_ID_MODEL:
        case Tag::ASSOCIATED_DATA:
        case Tag::CONFIRMATION_TOKEN:
        case Tag::NONCE:
            return a.blob == b.blob;

        /* Enum tags */
        case Tag::PURPOSE:
            return a.f.purpose == b.f.purpose;
        case Tag::ALGORITHM:
            return a.f.algorithm == b.f.algorithm;
        case Tag::BLOCK_MODE:
            return a.f.blockMode == b.f.blockMode;
        case Tag::DIGEST:
            return a.f.digest == b.f.digest;
        case Tag::PADDING:
            return a.f.paddingMode == b.f.paddingMode;
        case Tag::EC_CURVE:
            return a.f.ecCurve == b.f.ecCurve;
        case Tag::BLOB_USAGE_REQUIREMENTS:
            return a.f.keyBlobUsageRequirements == b.f.keyBlobUsageRequirements;
        case Tag::USER_AUTH_TYPE:
            return a.f.integer == b.f.integer;
        case Tag::ORIGIN:
            return a.f.origin == b.f.origin;
        case Tag::HARDWARE_TYPE:
            return a.f.hardwareType == b.f.hardwareType;
    }

    return false;
}

}  // namespace V4_0
}  // namespace keymaster
}  // namespace hardware
}  // namespace android

#endif  // SYSTEM_SECURITY_KEYSTORE_KEYMASTER_TAGS_H_
