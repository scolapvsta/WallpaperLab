#pragma once
#include <cstdint>
#include <cstddef>
// Minimal Linux flat-API ABI, checked against Valve's Steamworks headers
// (Steamworks.NET CodeGen/steam, ISteamUGC020). No SDK objects/vtables cross here.
namespace lab::steamabi {
#pragma pack(push,4)
struct Details {
 uint64_t id; int32_t result,type; uint32_t creatorApp,consumerApp;
 char title[129],description[8000]; uint64_t owner;
 uint32_t created,updated,added; int32_t visibility;
 bool banned,accepted,truncated; char tags[1025];
 uint64_t file,preview; char filename[260]; int32_t size,previewSize;
 char url[256]; uint32_t votesUp,votesDown; float score; uint32_t children;
 // Newer SDKs append fields. Reserve storage without reading unknown fields.
 uint64_t totalBytes; char reserved[256];
};
struct QueryResult {uint64_t handle;int32_t result;uint32_t returned,total;bool cached;char cursor[256];};
struct DownloadResult {uint32_t app;uint64_t id;int32_t result;};
struct SubscribeResult {int32_t result;uint64_t id;};
struct CallResult {uint64_t call;int32_t callback;uint32_t bytes;};
struct Callback {int32_t user,callback;unsigned char* data;int32_t size;};
#pragma pack(pop)
static_assert(offsetof(Details,owner)==8156);
static_assert(sizeof(QueryResult)==280);
static_assert(sizeof(DownloadResult)==16);
}
