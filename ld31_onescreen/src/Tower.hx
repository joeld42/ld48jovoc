import luxe.Mesh;
import luxe.Color;
import luxe.Vector;
import luxe.Quaternion;
import luxe.Input;
import luxe.Camera;
import luxe.Entity;
import luxe.Events;

import luxe.utils.Maths;


class Tower extends Entity
{
	public var mesh : Mesh;	
	public var towerName : String;

	public var shootTimeout : Float;
	public var shootDamage : Float;
	public var bulletSpeed : Float;
	public var shootRange : Float;
	var shootTimeleft : Float;

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
			shootTimeout = 1.0;
			shootDamage = 1.0;
			shootRange = 3.0;
			bulletSpeed = 20.0; // in grids per second
		}
		else // (towerName=='rock')
		{
			// Not a shooty
			shootTimeout = 0;
			shootDamage = 0;
			shootRange = 0;
			bulletSpeed = 1.0;
		}

		shootTimeleft = shootTimeout;
	}

	function do_shoot()
	{
		//trace('TODO: ${name} shoot...');
		Luxe.events.fire( "tower.shoot", this );

		// and reset
		shootTimeleft = shootTimeout;
	}

	override function update( dt : Float )
	{
		if (shootTimeout > 0.0)
		{
			shootTimeleft -= dt;
			if (shootTimeleft <= 0.0)
			{
				do_shoot();
			}
		}
	}
}
