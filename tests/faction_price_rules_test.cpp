#include "avatar.h"
#include "cata_catch.h"
#include "itype.h"
#include "npc.h"
#include "npctrade.h"
#include "player_helpers.h"

static const skill_id skill_speech( "speech" );

TEST_CASE( "faction_price_rules", "[npc][factions][trade]" )
{
    clear_avatar();
    npc &guy = spawn_npc( { 50, 50 }, "test_npc_trader" );
    faction const &fac = *guy.my_fac;

    WHEN( "item has no rules (default adjustment)" ) {
        item const hammer( "hammer" );
        clear_character( guy );
        REQUIRE( npc_trading::adjusted_price( &hammer, 1, get_avatar(), guy ) ==
                 Approx( units::to_cent( hammer.type->price_post ) * 1.25 ).margin( 1 ) );
        REQUIRE( npc_trading::adjusted_price( &hammer, 1, guy, get_avatar() ) ==
                 Approx( units::to_cent( hammer.type->price_post ) * 0.75 ).margin( 1 ) );
    }

    WHEN( "item is main currency (implicit price rule)" ) {
        guy.int_max = 1000;
        guy.set_skill_level( skill_speech, 10 );
        item const fmcnote( "FMCNote" );
        REQUIRE( npc_trading::adjusted_price( &fmcnote, 1, get_avatar(), guy ) ==
                 units::to_cent( fmcnote.type->price_post ) );
        REQUIRE( npc_trading::adjusted_price( &fmcnote, 1, guy, get_avatar() ) ==
                 units::to_cent( fmcnote.type->price_post ) );
    }

    WHEN( "item is secondary currency (fixed_adj=0)" ) {
        get_avatar().int_max = 1000;
        get_avatar().set_skill_level( skill_speech, 10 );
        item const pants_fur( "test_pants_fur" );
        REQUIRE( npc_trading::adjusted_price( &pants_fur, 1, get_avatar(), guy ) ==
                 units::to_cent( pants_fur.type->price_post ) );
        REQUIRE( npc_trading::adjusted_price( &pants_fur, 1, guy, get_avatar() ) ==
                 units::to_cent( pants_fur.type->price_post ) );
    }

    item const carafe( "test_nuclear_carafe" );
    WHEN( "condition for price rules not satisfied" ) {
        clear_character( guy );
        REQUIRE( fac.get_price_rules( carafe, guy ) == nullptr );
        REQUIRE( npc_trading::adjusted_price( &carafe, 1, get_avatar(), guy ) ==
                 Approx( units::to_cent( carafe.type->price_post ) * 1.25 ).margin( 1 ) );
    }
    WHEN( "condition for price rules satisfied" ) {
        guy.set_value( "npctalk_var_bool_allnighter_thirsty", "yes" );
        REQUIRE( fac.get_price_rules( carafe, guy )->markup == 2.0 );
        THEN( "NPC selling to avatar includes markup and positive fixed adjustment" ) {
            REQUIRE( npc_trading::adjusted_price( &carafe, 1, get_avatar(), guy ) ==
                     Approx( units::to_cent( carafe.type->price_post ) * 2.0 * 1.1 ).margin( 1 ) );
        }
        THEN( "avatar selling to NPC includes only negative fixed adjustment" ) {
            REQUIRE( npc_trading::adjusted_price( &carafe, 1, guy, get_avatar() ) ==
                     Approx( units::to_cent( carafe.type->price_post ) * 0.9 ).margin( 1 ) );
        }
    }
}
