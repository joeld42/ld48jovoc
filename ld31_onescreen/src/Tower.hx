import luxe.Mesh;
import luxe.Color;
import luxe.Vector;
import luxe.Quaternion;
import luxe.Input;
import luxe.Camera;
import luxe.Entity;
import luxe.Events;
import luxe.Sprite;

import luxe.utils.Maths;


class Tower extends Entity
{
	public var mesh : Mesh;	
	public var towerName : String;

	public var shootTimeout : Float;
	public var cardSpawnTimeout : Float;
	public var shootDamage : Float;
	public var bulletSpeed : Float;
	public var shootRange : Float;

	public var blocking : Bool;

	public var spawnCard : Sprite;

	var shootTimeleft : Float;
	var cardSpawnTimeleft :Float;

	public function new( _name : String, _mesh : Mesh )
	{
		super({
			name : _name,
			name_unique : true
			});

		towerName = _name;
		mesh = _mesh;

		configure();
	}

	function configure()
	{
		if (towerName=='snowman')
		{
			shootTimeout = 2.0;
			shootDamage = 4.0;
			shootRange = 3.0;
			bulletSpeed = 20.0; // in grids per second
		}
		else if (towerName=='barnacle')
		{
			shootTimeout = 0.5;
			shootDamage = 1.0;
			shootRange = 3.0;
			bulletSpeed = 15.0; // in grids per second
		}
		else // (towerName=='rock')
		{
			// Not a shooty
			shootTimeout = 0;
			shootDamage = 0;
			shootRange = 0;
			bulletSpeed = 1.0;
		}

		// is it blocking?
		if (towerName=='sring')
		{
			blocking = false;
		}
		else
		{
			blocking = true;
		}

		// Does it spawn cards?
		if ((towerName=='sring') || (towerName=='sstone'))
		{
			cardSpawnTimeout = 5.0;
		}
		else
		{
			cardSpawnTimeout = 0;
		}

		shootTimeleft = shootTimeout;
		cardSpawnTimeleft = cardSpawnTimeout;
	}

	function do_shoot()
	{
		//trace('TODO: ${name} shoot...');
		Luxe.events.fire( "tower.shoot", this );

		// and reset
		shootTimeleft = shootTimeout;
	}

	function do_spawnCard()
	{
		Luxe.events.fire( "tower.spawncard", this );		
		cardSpawnTimeleft = cardSpawnTimeout;
	}

	override function update( dt : Float )
	{
		// Shoot timer
		if (shootTimeout > 0.0)
		{
			shootTimeleft -= dt;
			if (shootTimeleft <= 0.0)
			{
				do_shoot();
			}
		}


		// Card spawn timer
		if ((cardSpawnTimeout > 0.0) && (spawnCard==null))
		{
			cardSpawnTimeleft -= dt;
			if (cardSpawnTimeleft <= 0.0)
			{
				do_spawnCard();
			}
		}
	}
}
