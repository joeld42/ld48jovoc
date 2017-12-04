
// FIXME: clean up and make this a proper include
void
TestApp::populateCustomers( Oryol::Array<CrateItemInfo> &allItems )
{
    
    
    allItems.Add( CrateItemInfo( "Wooden Crate", ItemShape_CRATE,
                                10, 8, // Point, Payment
                                Decal_CRATE, glm::vec4(0.99,0.82,0.66,1.0), glm::vec4(1,1,1,1),
                                60, 300, // Pickup min/max
                                "Boss said to drop off some crates here.",
                                "I need a crate, just a wooden one is fine.",
                                "Nice work, you're pretty efficient around here.",
                                "This crate will do nicely.",
                                "Man, took you that long to find a crate in a video game?" ) );
    
    allItems.Add( CrateItemInfo( "Hardwood Box", ItemShape_CRATE,
                                25, 11, // Point, Payment
                                Decal_CRATE, glm::vec4(0.61,0.39,0.32,1.0), glm::vec4(0.2,0,0,1),
                                120, 600, // Pickup min/max
                                "This box is made of the finest endangered jungle spruce.",
                                "Please retrieve my well-crafted encosure.",
                                "Very quick. I can't wait until everything is automated.",
                                "Very good.",
                                "That took forever. I'm speaking to your manager." ) );
    
    
    allItems.Add( CrateItemInfo( "LD48 Crate", ItemShape_CRATE,
                                20, 12, // Point, Payment
                                Decal_LD48, glm::vec4(0.91,0.33,0.24,1.0), glm::vec4(0.95,0.56,0.22,1),
                                60, 300, // Pickup min/max
                                "Check out my LDJAM commemorative Loot Crate.",
                                "I'm checking on my valuable LDJAM Loot Crate.",
                                "That was quick, you should make a game new LDJAM.",
                                "Excellent. Now go rate some games.",
                                "Yikes, what happened, server issues?" ) );
    
    
    allItems.Add( CrateItemInfo( "Barrel", ItemShape_BARREL,
                                10, 5, // Point, Payment
                                Decal_LD48, glm::vec4(0.40,0.49,0.60,1.0), glm::vec4(0,0,0,0),
                                60, 300, // pickup min/max
                                "Got a barrel for you. It's just a regular barrel.",
                                "I'll be needing a barrel, thanks.",
                                "That was fast, thanks. I've got time to grab a sandwich.",
                                "Yep, that's a barrel.",
                                "Jeez, how long does it take to find a stinkin barrel?" ) );

    allItems.Add( CrateItemInfo( "Barrel LD48", ItemShape_BARREL,
                                10, 5, // Point, Payment
                                Decal_LD48, glm::vec4(0.65,0.23,0.84,1.0), glm::vec4(1,1,1,1),
                                60, 300, // pickup min/max
                                "It's a barrel... of JAM!",
                                "You got my barrel? It says LD48 on the side.",
                                "Awesome, thanks buddy.",
                                "Yep, that's my jammy barrel.",
                                "Sigh. I've been here all day." ) );

    
    allItems.Add( CrateItemInfo( "Fire Barrel", ItemShape_BARREL,
                                20, 15, // Point, Payment
                                Decal_FLAMMABLE, glm::vec4(0.74,0.28,0.33,1.0), glm::vec4(1,1,1,1),
                                60, 300, // pickup min/max
                                "Watch out, it's highly flammable.",
                                "I need a flamming barrel for entirely innocuous reasons.",
                                "Smokin! Get it? Like, you were fast, but also, smoke, fire...",
                                "Thanks. If anyone asks, I wasn't here.",
                                "I was worried there that you'd blown yourself up." ) );
    
    allItems.Add( CrateItemInfo( "Dynamite", ItemShape_CRATE,
                                30, 25, // Point, Payment
                                Decal_FLAMMABLE2, glm::vec4(0.74,0.28,0.33,1.0), glm::vec4(1,1,1,1),
                                120, 300, // pickup min/max
                                "Here's some dynamite. Don't drop it.",
                                "Do you have my dynamite? I'm starting a new mine.",
                                "You got it! Dyn-o-mite!",
                                "Thanks. Off to blast some earth and stone.",
                                "Slooow. You don't have to carry it that carefully." ) );
    
    
}
