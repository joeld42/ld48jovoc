import luxe.Mesh;
import luxe.Color;
import luxe.Vector;
import luxe.Quaternion;
import luxe.Input;
import luxe.Camera;
import luxe.Entity;
import luxe.utils.Maths;

class Tower extends Entity
{
	public var mesh : Mesh;	
	public var towerName : String;

	public function new( _name : String, _mesh : Mesh )
	{
		super({
			name : _name,
			name_unique : true
			});

		towerName = _name;
		mesh = _mesh;
	}
}
