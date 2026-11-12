#pragma once
namespace cfghostedbudget {
// Read-only selection/catalogue proofs repeat listen-host and ownership checks.
// Native dispatch still uses its existing 1536-read / 12-call retained gateway.
constexpr unsigned EmoteReads=4096;
constexpr unsigned EmoteReadCap(bool hosted,unsigned offline){return hosted?EmoteReads:offline;}
}
