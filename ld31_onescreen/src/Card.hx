import phoenix.Batcher;
import luxe.Vector;
import luxe.Sprite;
import luxe.Input;
import luxe.tween.Actuate;

class Card extends Sprite
{
	@:isVar public var handNdx (default, set) : Int = 0;

	// position and rotation when held in hand
	var handPos : Vector;
	var handRot : Float;
	public var lifted : Bool; // picked up but not yet placing
	public var placing : Bool;
	var flipRot : Float;
	public var topname : String;
	public var flipname : String;
	var flipSprite : Sprite;

	public function new( _topname : String, _flipname : String, batcher : Batcher ) 
	{
		// card
		topname = _topname;
		flipname = _flipname;

    	 // load the test sprite
    	 var cardSz = 200.0;
    	 var cardTex = Luxe.loadTexture( 'assets/card_${_topname}.png' );

		//create visual
        super( {
	            name: 'card',
	            name_unique: true,
	            texture: cardTex,
	            batcher : batcher,
	            pos: new Vector( 900, 400 ),
	            size: new Vector( cardSz * 0.7117, cardSz ),
	            depth: -5
			});

        // Child sprite for the bottom half of the card
        var flipTex = Luxe.loadTexture( 'assets/card_${_flipname}.png' );
        flipSprite = new Sprite({
	            name: 'cardflip',
	            name_unique: true,
	            texture: flipTex,
	            batcher : batcher,
	            pos: new Vector( 900, 400 ),
	            size: new Vector( cardSz * 0.7117, cardSz ),
	            rotation_z : 180.0,
	            depth: -4.1
			});

        // Hmm neither way of parenting works with scaling, \
        // have to duplicate all the tweens, ugh..
        //flipSprite.parent = this;
        // flipSprite.transform.parent = this.transform;
	}

	public function destroyAll()
	{
		flipSprite.destroy();
		this.destroy();
	}

	function set_handNdx( _handNdx : Int ) : Int
	{
		handNdx = _handNdx;

		handPos = new Vector( 250 + _handNdx*(600/7.0), 520 - (Math.sin( (3.1415/6)*_handNdx )) * 50 );
		handRot = -15 + ((30.0 / 7) * _handNdx);

		flipRot = 0.0;
		depth = -5 - handNdx;
		flipSprite.depth = -4.1 - handNdx;

		Actuate.tween( this.pos, 0.2, { x : handPos.x, y : handPos.y  } );
		Actuate.tween( this, 0.2, { rotation_z : handRot } );

		Actuate.tween( flipSprite.pos, 0.2, { x : handPos.x, y : handPos.y  } );
		Actuate.tween( flipSprite, 0.2, { rotation_z : handRot } );

		return handNdx;
	}

	public function returnToHand()
	{
		 // Actuate.tween( distAmt, 0.2, { x : 1.0 } )
		 pos = handPos.clone();
		 rotation_z = handRot;		 

		 flipSprite.pos = handPos.clone();
		 flipSprite.rotation_z = handRot;		 
	}

	override function init() {
        // trace('\tSprite init');
    } //init

    // Hmm not sure how this works
 //    override function onmousedown(e:MouseEvent) {
 //    	trace('Card mousedown');
 //        // mouse = e.pos;
 //        // dragging = true;    
 //    }

	// override function onmouseup(e:MouseEvent) {
 //    	trace('Card mousedown');
 //        // mouse = e.pos;
 //        // dragging = true;    
 //    }

   	public function fakemousedown(e:MouseEvent) {

   		if (e.button == MouseButton.left)
   		{
	    	// trace('Card mousedown');
	    	lifted = true;
	    	var cardSzY = 400.0;
			var cardSzX = cardSzY * 0.7117;

	        Actuate.tween( pos, 0.2, { x : 500, y : 300 } );
	        Actuate.tween( flipSprite.pos, 0.2, { x : 500, y : 300 } );
	       	        
	        Actuate.tween( size, 0.4, { x : cardSzX, y : cardSzY  } );
	        Actuate.tween( flipSprite.size, 0.4, { x : cardSzX, y : cardSzY  } );
	        
	        Actuate.tween( this, 0.2, { rotation_z : flipRot } );
	        Actuate.tween( flipSprite, 0.2, { rotation_z : flipRot } );

	        depth = 0;
	        flipSprite.depth = 0.1;

	        // Actuate.tween( rotation_z, 0.2, { rotation_z : 0 } );
    	}
    	else if (e.button == MouseButton.right)
    	{
    		if (flipRot < 10.0)
    		{
    			flipRot = 180.0;
    		}
    		else
    		{
    			flipRot = 0.0;
    		}

    		// TODO: tween
    		if (lifted)
    		{
    			// rotation_z = flipRot;
    			Actuate.tween( this, 0.2, { rotation_z : flipRot } );
    			Actuate.tween( flipSprite, 0.2, { rotation_z : flipRot } );
    		}
    		else
    		{
    			// rotation_z = handRot + flipRot;	
    			Actuate.tween( this, 0.2, { rotation_z : handRot + flipRot } );
    			Actuate.tween( flipSprite, 0.2, { rotation_z : handRot + flipRot } );
    		}

    	}
    }

	public function fakemouseup(e:MouseEvent) {
    	 

    	if (e.button == MouseButton.left)
   		{
	        // mouse = e.pos;
	        // dragging = true;    
	        placing = false;
	        if (lifted)
	        {
	        	var cardSz = 200.0;
	        	Actuate.tween( pos, 0.2, { x : handPos.x, y : handPos.y } );
	        	Actuate.tween( flipSprite.pos, 0.2, { x : handPos.x, y : handPos.y } );

				Actuate.tween( size, 0.4, { x : cardSz * 0.7117, y : cardSz  } );
	        	Actuate.tween( flipSprite.size, 0.4, { x : cardSz * 0.7117, y : cardSz  } );

	        	Actuate.tween( color, 0.3, { a : 1.0 } );
	        	Actuate.tween( flipSprite.color, 0.3, { a : 1.0 } );
	        	
	        	Actuate.tween( this, 0.2, { rotation_z : handRot + flipRot } );
	        	Actuate.tween( flipSprite, 0.2, { rotation_z : handRot + flipRot } );

	        	depth = -5 - handNdx;
	        	flipSprite.depth = -4.1 - handNdx;
	        	lifted = false;
	    	}
    	}
    }

    public function fakemousemove(e:MouseEvent) {

    	if ((e.y < 400) || (placing))
    	{
    		if (!placing)
    		{
    			placing = true;
				var cardSz = 80.0;
				Actuate.tween( size, 0.4, { x : cardSz * 0.7117, y : cardSz  } );
				Actuate.tween( flipSprite.size, 0.4, { x : cardSz * 0.7117, y : cardSz  } );

				Actuate.tween( color, 0.3, { a : 0.3 } );
				Actuate.tween( flipSprite.color, 0.3, { a : 0.3 } );
    		}

    		pos.set_xy( e.x, e.y );
    		flipSprite.pos.set_xy( e.x, e.y );
    	}

    }

}