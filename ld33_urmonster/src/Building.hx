import luxe.Entity;
import luxe.Color;
import luxe.Vector;
import luxe.Sprite;
import luxe.Camera;

import phoenix.Rectangle;
import phoenix.Batcher;

import luxe.tween.Actuate;

import SceneRender;

class Building 
{
	public var entity_ : Entity;
	public var sceneObj_ : SceneObj;
	public var healthBar_ : Sprite;
	public var type_ : String;

	public var maxHealth_ : Float;
	public var health_ : Float;

	var damageFade_ : Float;

	public function new( name : String, type : String, obj : SceneObj, batcher : Batcher )
	{
		// make an entity for this
 		entity_ = new Entity({ name: name });
    	entity_.transform = obj.xform_;

    	trace('New Building: ${name} ${type}');
    	sceneObj_ = obj;

    	type_ = type;
    	if (type=="House") {
    		maxHealth_ = 3.0;
    	} else if (type=="Tree") {
    		maxHealth_ = 5.0;	
		} else if (type=="Office") {
    		maxHealth_ = 8.0;
		} else if (type=="Tower") {
			maxHealth_ = 20.0;
    	} else {
    		maxHealth_ = 1.0;
    		trace('WARNING: Unkown building type ${type}');
    	}
    	health_ = maxHealth_;



    	var sz = 1.0;
 		healthBar_ = new Sprite({
 			name: name + ".health",
 			pos : new Vector( 0, 0, 0 ),
 			size : new Vector( sz, sz/8.0 ),
 			texture : Luxe.resources.texture("assets/healthbar_half.png"),
 			uv : new Rectangle( 0.0, 0.0, 128, 32 ),
 			batcher: batcher
 			});

 		healthBar_.pos.copy_from( sceneObj_.xform_.pos);
 		
 		if (type=="Tower") {			
 			healthBar_.pos.y += 10.0;
 			healthBar_.pos.z -= 5.0;
		} else {
			healthBar_.pos.y += sceneObj_.boundSphere_.radius_ * 1.2;
 			healthBar_.pos.z -= 1.0;
 		}
	}

	public function checkCollide( newpos : Vector, radius : Float ) : Bool
	{
		// FIXME: this doesn't take local centers into account
		var d = Vector.Subtract(newpos, sceneObj_.xform_.pos ).length;
		return (d < sceneObj_.boundSphere_.radius_ + radius );
	}

	public function takeDamage( amount : Float ) : Bool
	{		
		damageFade_ = 1.0;

		health_ -= amount;
		if (health_ < 0.0)
		{			
			// we're dead
			damageFade_ = 0.0;
			sceneObj_.tintColor.set_xyz( 0.2, 0.2, 0.2 );
			sceneObj_.jitter_ = 0.0;
			healthBar_.destroy();
			return true;
		}
		return false;
	}

	public function update( dt : Float, gameCamera : Camera )
	{
		if (damageFade_ > 0.0)
		{
			damageFade_ -= dt * 5.0;
			if (damageFade_ < 0.0)
			{
				damageFade_ = 0.0;
			}
			sceneObj_.tintColor.set_xyz( 1.0, 1.0, 1.0 ).lerp_xyz( 1.0, 0.0, 0.0, damageFade_ );
			sceneObj_.jitter_ = damageFade_ * 0.2;
		}

		// update health bar
		var pct = health_ / maxHealth_;
		healthBar_.uv.x = 128 - (pct*128);
    	healthBar_.rotation.copy( gameCamera.rotation );

	}


}