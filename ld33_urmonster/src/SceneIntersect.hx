import Math;

import luxe.Vector;

import phoenix.geometry.Geometry;
import phoenix.geometry.Vertex;
import phoenix.geometry.TextureCoord;

import phoenix.Transform;
import phoenix.Matrix;

class SceneRay {
	public var origin_ : Vector;
	public var dir_ : Vector;	
	public function new ( origin : Vector, dir : Vector )
	{
		origin_ = origin;
		dir_ = dir;
	}

	inline function toString() {

        return "{ origin:"+ origin_ + ", dir:" + dir_ + " }" ;

    } //toString

    public function intersectPlane( planeN : Vector, planeP : Vector ) : SceneRayResult
    {
    	var result = new SceneRayResult();
    	var dd = planeN.dot( dir_ );
    	if (Math.abs(dd) > 0.0001)
    	{
    		var t : Float = Vector.Subtract( planeP, origin_ ).dot( planeN ) / dd;
    		if ( t >= 0.0) {
    			result.hit_ = true;
    			result.hitNormal_.copy_from(  planeN );
    			result.hitPoint_ = Vector.Add( origin_, dir_.clone().multiplyScalar(t) );
    		}
    	} else {
    		result.hit_ = false;
    	}
    	return result;
    }

}

class SceneRayResult
{
	public var hit_ : Bool;
	public var hitPoint_ : Vector;
	public var hitNormal_: Vector;

	public function new ()
	{
		hit_ = false;
		hitPoint_ = new Vector();
		hitNormal_ = new Vector();
	}
}

class SceneBoundSphere
{
	public var center_ : Vector;
	public var radius_ : Float;
	public function new ( center : Vector, radius : Float )
	{
		center_ = center;
		radius_ = radius;
	}

	inline function toString() {

        return "{ center:"+ center_ + ", radius:" + radius_ + " }" ;

    } //toString

	public static function BoundsFromGeom( geom : Geometry )
	{
		var center = new Vector();
		var radius = 0.0;

		if (geom.vertices.length > 0 )
		{
			for (v in geom.vertices)
			{
				center.add( v.pos );			
			}

			center.multiplyScalar( 1.0 / geom.vertices.length );			

			for (v in geom.vertices)
			{
				var d = Vector.Subtract( v.pos, center ).length;
				if ( d > radius) {
					radius = d;
				}
			}
		}

		return new SceneBoundSphere( center, radius );
	}

	// Adapted from: http://paulbourke.net/geometry/circlesphere/raysphere.c
	public function intersectRay( ray : SceneRay ) : SceneRayResult
	{
		var res = new SceneRayResult();
		var a = ray.dir_.dot( ray.dir_ );
		var rc = Vector.Subtract( ray.origin_, center_ );
		var b = 2 * ray.dir_.dot( rc );
		var c = center_.dot( center_ );
		c += ray.origin_.dot( ray.origin_ );
		c -= 2 * center_.dot( ray.origin_ );
		c -= radius_ * radius_;
		var bb4ac = b * b - 4 * a * c;
		if ((Math.abs(a)) < 0.0001 || (bb4ac < 0)) 
		{
			res.hit_ = false;
			return res;
		}

		// *mu1 = (-b + sqrt(bb4ac)) / (2 * a);
		// *mu2 = (-b - sqrt(bb4ac)) / (2 * a);
		var mu2 = (-b - Math.sqrt(bb4ac)) / (2 * a);
		res.hitPoint_ = Vector.Add( ray.origin_, ray.dir_.clone().multiplyScalar( mu2 ) );
		res.hitPoint_.w = 1.0;

		res.hitNormal_ = Vector.Subtract( res.hitPoint_, center_ );
		res.hitNormal_.w = 0.0;
		res.hitNormal_.normalize();


		res.hit_ = true;
		return res;
	}


}

