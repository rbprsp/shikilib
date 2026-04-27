#include "db/db.h"
#include "mapping/resolver.h"
#include "models/anilib.h"
#include "models/shiki.h"

#include <catch2/catch_test_macros.hpp>

using N = MappingResolver;

namespace
{

Anime MakeAnime(int id, std::string name, std::string eng = "", std::string rus = "")
{
    Anime a;
    a.id = id;
    a.name = std::move(name);
    a.eng_name = std::move(eng);
    a.rus_name = std::move(rus);
    a.type = "tv";
    a.slug = "slug-" + std::to_string(id);
    a.slug_url = a.slug;
    return a;
}

ShikiList MakeShiki(int id, std::string title, std::string title_ru = "")
{
    ShikiList s;
    s.target_id = id;
    s.target_title = std::move(title);
    s.target_title_ru = std::move(title_ru);
    s.target_type = "Anime";
    s.score = 0;
    s.status = "completed";
    s.rewatches = 0;
    s.episodes = 0;
    return s;
}

} // namespace

// ---- NormalizeName ----------------------------------------------------------

TEST_CASE("normalize: empty input", "[normalize]")
{
    REQUIRE(N::NormalizeName("") == "");
}

TEST_CASE("normalize: ascii lowercased", "[normalize]")
{
    REQUIRE(N::NormalizeName("HelloWorld") == "helloworld");
    REQUIRE(N::NormalizeName("FULLMETAL") == "fullmetal");
}

TEST_CASE("normalize: punctuation stripped", "[normalize]")
{
    REQUIRE(N::NormalizeName("Re:Zero!") == "rezero");
    REQUIRE(N::NormalizeName("Steins;Gate.") == "steinsgate");
    REQUIRE(N::NormalizeName("K-On!!") == "kon");
}

TEST_CASE("normalize: whitespace stripped", "[normalize]")
{
    REQUIRE(N::NormalizeName("Hello World") == "helloworld");
    REQUIRE(N::NormalizeName("  spaced  out  ") == "spacedout");
    REQUIRE(N::NormalizeName("tab\there") == "tabhere");
}

TEST_CASE("normalize: digits preserved", "[normalize]")
{
    REQUIRE(N::NormalizeName("Steins;Gate 0") == "steinsgate0");
    REQUIRE(N::NormalizeName("86") == "86");
}

TEST_CASE("normalize: equivalence after stripping", "[normalize]")
{
    REQUIRE(N::NormalizeName("Fullmetal Alchemist: Brotherhood") ==
            N::NormalizeName("Fullmetal Alchemist Brotherhood"));
    REQUIRE(N::NormalizeName("Cowboy Bebop") == N::NormalizeName("cowboy  bebop"));
}

TEST_CASE("normalize: cyrillic preserved", "[normalize]")
{
    auto out = N::NormalizeName("Осторожный герой");
    REQUIRE_FALSE(out.empty());
    REQUIRE(out == N::NormalizeName("Осторожный  герой!"));
}

TEST_CASE("normalize: japanese preserved", "[normalize]")
{
    auto out = N::NormalizeName("鋼の錬金術師");
    REQUIRE_FALSE(out.empty());
    REQUIRE(out == N::NormalizeName("鋼 の 錬 金 術 師"));
}

TEST_CASE("normalize: idempotent", "[normalize]")
{
    auto once = N::NormalizeName("Re:Zero!");
    auto twice = N::NormalizeName(once);
    REQUIRE(once == twice);
}

TEST_CASE("normalize: only punctuation -> empty", "[normalize]")
{
    REQUIRE(N::NormalizeName("!!!---...") == "");
    REQUIRE(N::NormalizeName("   ") == "");
}

// ---- ResolveByName ----------------------------------------------------------

TEST_CASE("resolver: empty shiki db -> all zero outcome", "[resolver]")
{
    AnilibDB anilib(":memory:");
    ShikiDB shiki(":memory:");
    MappingDB mapping(":memory:");

    MappingResolver r(mapping, anilib);
    auto out = r.ResolveByName(shiki);

    REQUIRE(out.matched == 0);
    REQUIRE(out.ambiguous == 0);
    REQUIRE(out.missed == 0);
    REQUIRE(out.cached == 0);
    REQUIRE(out.missed_entries.empty());
}

TEST_CASE("resolver: exact name match by target_title", "[resolver]")
{
    AnilibDB anilib(":memory:");
    ShikiDB shiki(":memory:");
    MappingDB mapping(":memory:");

    anilib.GetStorage().replace(MakeAnime(1, "Cowboy Bebop"));
    shiki.GetStorage().replace(MakeShiki(100, "Cowboy Bebop"));

    MappingResolver r(mapping, anilib);
    auto out = r.ResolveByName(shiki);

    REQUIRE(out.matched == 1);
    REQUIRE(out.missed == 0);

    auto stored = mapping.GetStorage().get_pointer<Mapping>(100);
    REQUIRE(stored != nullptr);
    REQUIRE(stored->anilib_id.has_value());
    REQUIRE(*stored->anilib_id == 1);
    REQUIRE(stored->name == "Cowboy Bebop");
}

TEST_CASE("resolver: match through eng_name", "[resolver]")
{
    AnilibDB anilib(":memory:");
    ShikiDB shiki(":memory:");
    MappingDB mapping(":memory:");

    anilib.GetStorage().replace(MakeAnime(7, "Hagane no Renkinjutsushi", "Fullmetal Alchemist: Brotherhood"));
    shiki.GetStorage().replace(MakeShiki(5114, "Fullmetal Alchemist Brotherhood"));

    MappingResolver r(mapping, anilib);
    auto out = r.ResolveByName(shiki);

    REQUIRE(out.matched == 1);
    auto stored = mapping.GetStorage().get_pointer<Mapping>(5114);
    REQUIRE(stored != nullptr);
    REQUIRE(stored->anilib_id == 7);
}

TEST_CASE("resolver: match through rus_name fallback", "[resolver]")
{
    AnilibDB anilib(":memory:");
    ShikiDB shiki(":memory:");
    MappingDB mapping(":memory:");

    anilib.GetStorage().replace(MakeAnime(42, "Shinchou Yuusha", "", "Осторожный герой"));
    shiki.GetStorage().replace(MakeShiki(38659, "TotallyDifferentRomaji", "Осторожный герой"));

    MappingResolver r(mapping, anilib);
    auto out = r.ResolveByName(shiki);

    REQUIRE(out.matched == 1);
    auto stored = mapping.GetStorage().get_pointer<Mapping>(38659);
    REQUIRE(stored->anilib_id == 42);
}

TEST_CASE("resolver: no match -> missed entry recorded", "[resolver]")
{
    AnilibDB anilib(":memory:");
    ShikiDB shiki(":memory:");
    MappingDB mapping(":memory:");

    anilib.GetStorage().replace(MakeAnime(1, "Some Other Anime"));
    shiki.GetStorage().replace(MakeShiki(999, "Nonexistent Title"));

    MappingResolver r(mapping, anilib);
    auto out = r.ResolveByName(shiki);

    REQUIRE(out.missed == 1);
    REQUIRE(out.matched == 0);
    REQUIRE(out.missed_entries.size() == 1);
    REQUIRE(out.missed_entries.front().target_id == 999);

    auto stored = mapping.GetStorage().get_pointer<Mapping>(999);
    REQUIRE(stored != nullptr);
    REQUIRE_FALSE(stored->anilib_id.has_value());
}

TEST_CASE("resolver: ambiguous candidates -> not locked, recorded for review", "[resolver]")
{
    AnilibDB anilib(":memory:");
    ShikiDB shiki(":memory:");
    MappingDB mapping(":memory:");

    anilib.GetStorage().replace(MakeAnime(1, "Fullmetal Alchemist"));
    anilib.GetStorage().replace(MakeAnime(2, "Fullmetal Alchemist"));
    shiki.GetStorage().replace(MakeShiki(5114, "Fullmetal Alchemist"));

    MappingResolver r(mapping, anilib);
    auto out = r.ResolveByName(shiki);

    REQUIRE(out.ambiguous == 1);
    REQUIRE(out.matched == 0);
    REQUIRE(out.ambiguous_entries.size() == 1);
    REQUIRE(out.ambiguous_entries.front().target_id == 5114);

    auto stored = mapping.GetStorage().get_pointer<Mapping>(5114);
    REQUIRE(stored != nullptr);
    REQUIRE_FALSE(stored->anilib_id.has_value());
}

TEST_CASE("resolver: ambiguous re-tried on second run", "[resolver]")
{
    AnilibDB anilib(":memory:");
    ShikiDB shiki(":memory:");
    MappingDB mapping(":memory:");

    anilib.GetStorage().replace(MakeAnime(1, "Fullmetal Alchemist"));
    anilib.GetStorage().replace(MakeAnime(2, "Fullmetal Alchemist"));
    shiki.GetStorage().replace(MakeShiki(5114, "Fullmetal Alchemist"));

    MappingResolver r(mapping, anilib);
    auto first = r.ResolveByName(shiki);
    REQUIRE(first.ambiguous == 1);
    REQUIRE(first.cached == 0);

    auto second = r.ResolveByName(shiki);
    REQUIRE(second.ambiguous == 1);
    REQUIRE(second.cached == 0);
    REQUIRE(second.ambiguous_entries.size() == 1);
}

TEST_CASE("resolver: cache hit -> skipped", "[resolver]")
{
    AnilibDB anilib(":memory:");
    ShikiDB shiki(":memory:");
    MappingDB mapping(":memory:");

    anilib.GetStorage().replace(MakeAnime(1, "Cowboy Bebop"));
    shiki.GetStorage().replace(MakeShiki(100, "Cowboy Bebop"));

    Mapping prev;
    prev.shiki_id = 100;
    prev.anilib_id = 999;
    prev.name = "manually fixed";
    mapping.GetStorage().replace(prev);

    MappingResolver r(mapping, anilib);
    auto out = r.ResolveByName(shiki);

    REQUIRE(out.cached == 1);
    REQUIRE(out.matched == 0);

    auto stored = mapping.GetStorage().get_pointer<Mapping>(100);
    REQUIRE(stored->anilib_id == 999);
    REQUIRE(stored->name == "manually fixed");
}

TEST_CASE("resolver: cached but anilib_id null -> retry", "[resolver]")
{
    AnilibDB anilib(":memory:");
    ShikiDB shiki(":memory:");
    MappingDB mapping(":memory:");

    anilib.GetStorage().replace(MakeAnime(1, "Cowboy Bebop"));
    shiki.GetStorage().replace(MakeShiki(100, "Cowboy Bebop"));

    Mapping pending;
    pending.shiki_id = 100;
    pending.anilib_id = std::nullopt;
    pending.name = "Cowboy Bebop";
    mapping.GetStorage().replace(pending);

    MappingResolver r(mapping, anilib);
    auto out = r.ResolveByName(shiki);

    REQUIRE(out.matched == 1);
    REQUIRE(out.cached == 0);

    auto stored = mapping.GetStorage().get_pointer<Mapping>(100);
    REQUIRE(stored->anilib_id == 1);
}

TEST_CASE("resolver: mixed batch -> correct counts", "[resolver]")
{
    AnilibDB anilib(":memory:");
    ShikiDB shiki(":memory:");
    MappingDB mapping(":memory:");

    anilib.GetStorage().replace(MakeAnime(1, "Cowboy Bebop"));
    anilib.GetStorage().replace(MakeAnime(2, "Trigun"));
    anilib.GetStorage().replace(MakeAnime(3, "Death Note"));
    anilib.GetStorage().replace(MakeAnime(4, "Death Note"));

    shiki.GetStorage().replace(MakeShiki(10, "Cowboy Bebop"));
    shiki.GetStorage().replace(MakeShiki(20, "Trigun"));
    shiki.GetStorage().replace(MakeShiki(30, "Death Note"));
    shiki.GetStorage().replace(MakeShiki(40, "Unknown Anime"));

    MappingResolver r(mapping, anilib);
    auto out = r.ResolveByName(shiki);

    REQUIRE(out.matched == 2);
    REQUIRE(out.ambiguous == 1);
    REQUIRE(out.missed == 1);
    REQUIRE(out.missed_entries.size() == 1);
    REQUIRE(out.missed_entries.front().target_id == 40);
    REQUIRE(out.ambiguous_entries.size() == 1);
    REQUIRE(out.ambiguous_entries.front().target_id == 30);
}

TEST_CASE("resolver: title with punctuation matches normalized", "[resolver]")
{
    AnilibDB anilib(":memory:");
    ShikiDB shiki(":memory:");
    MappingDB mapping(":memory:");

    anilib.GetStorage().replace(MakeAnime(1, "Re:Zero kara Hajimeru Isekai Seikatsu"));
    shiki.GetStorage().replace(MakeShiki(100, "Re Zero kara Hajimeru Isekai Seikatsu!"));

    MappingResolver r(mapping, anilib);
    auto out = r.ResolveByName(shiki);

    REQUIRE(out.matched == 1);
    auto stored = mapping.GetStorage().get_pointer<Mapping>(100);
    REQUIRE(stored->anilib_id == 1);
}
