
import luxe.Vector;

import luxe.Component;
import luxe.Entity;
import phoenix.Quaternion;

//This component will rotate the entity that it is attached to a small amount each frame.
//It is assumed that the entity is a Sprite! 

class Rotate extends Component {


	public var rotate_speed : Float = 10;
	public var max_rotate_speed : Float = 60;    

    override function update( dt:Float ) {

    		//changes to the transform inside of components affect the entity directly!
        var q = new Quaternion();
        q.setFromAxisAngle( new Vector( 1.0, 0.0, 0.0), 1.2*dt );

        entity.rotation.multiply(q);

    } //update

} //Rotate
