
import luxe.Vector;

import luxe.Component;
import luxe.Entity;
import phoenix.Quaternion;

class ZillaMover extends Component 
{
	// public var rotate_speed : Float = 10;
	// public var max_rotate_speed : Float = 60;    

	public var forceUp_ : Float = 0.0;
	public var forceDown_ : Float = 0.0;
	public var forceLeft_ : Float = 0.0;
	public var forceRight_ : Float = 0.0;

    override function update( dt:Float ) {

    	//trace('up ${forceUp_} down ${forceDown_} left ${forceLeft_} right ${forceRight_}');
    	
   
        var fwd = entity.transform.world.matrix.forward();
        fwd.normalize();
        fwd.multiplyScalar( forceUp_ - forceDown_ );

        // var right = entity.transform.world.matrix.right();
        // right.normalize();
        // right.multiplyScalar( forceRight_ - forceLeft_ );

        var moveDir = fwd.clone();
        moveDir.normalize();

        moveDir.multiplyScalar( dt * 20.0 );
        entity.pos.add( moveDir ); 

        var q = new Quaternion();
     	q.setFromAxisAngle( new Vector( 0.0, 1.0, 0.0), (forceLeft_-forceRight_)*4.0*dt );
        entity.rotation.multiply(q);


    } //update

} //Rotate