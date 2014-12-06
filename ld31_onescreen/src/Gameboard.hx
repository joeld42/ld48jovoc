import luxe.Mesh;
import luxe.Color;
import luxe.Vector;
import luxe.Quaternion;
import luxe.Input;
import luxe.Camera;
import phoenix.Ray;
import luxe.utils.Maths;

class BoardCell
{

}

class Gameboard
{
	var mesh : Mesh;
	var cursor : Mesh;
	var snowman: Mesh;

	public var cursorPos : Vector;
	public var snapCursorPos : Vector; // cursor pos snapped to a grid cell

	var size : Int;
	var  cells : Array<BoardCell>;

	public function new()
	{
		size = 5;
		cells = new Array<BoardCell>();

		cursorPos = new Vector();
		snapCursorPos = new Vector();
	}

	public function setup( _onload : Gameboard -> Void )
	{
    	// Load the gameboard
		var tex = Luxe.loadTexture('assets/gameboard_5x5.png');     
        tex.clamp = repeat;  
        tex.onload = function(t) 
        {
            new Mesh({ file:'assets/gameboard_5x5.obj', 
                      texture: t, 
                      onload : function ( m : Mesh ) {
                            
                            mesh = m;
                            // gameboard.geometry.shader = worldShader;

                            //loadCount++;
                            _onload( this );

                            //gameboard.pos.set_xyz( 0.0, -5.0, 0.0 );
                        }
                      });
        }

		// Load the gameboard
		var tex2 = Luxe.loadTexture('assets/testgrid.png');     
        tex2.clamp = repeat;  
        tex2.onload = function(t) 
        {
            new Mesh({ file:'assets/cursor.obj', 
                      texture: t, 
                      onload : function ( m : Mesh ) {
                            
                            cursor = m;
                            // gameboard.geometry.shader = worldShader;

                            //loadCount++;
                            _onload( this );

                            //gameboard.pos.set_xyz( 0.0, -5.0, 0.0 );
                        }
                      });
        }

        // Load the snowman
		var tex2 = Luxe.loadTexture('assets/testgrid.png');     
        tex2.clamp = repeat;  
        tex2.onload = function(t) 
        {
            new Mesh({ file:'assets/snowman.obj', 
                      texture: t, 
                      onload : function ( m : Mesh ) {
                            
                            snowman = m;
                            // gameboard.geometry.shader = worldShader;

                            //loadCount++;
                            _onload( this );

                            //gameboard.pos.set_xyz( 0.0, -5.0, 0.0 );
                        }
                      });
        }
	}

	function cell( x : Int, y : Int ) : BoardCell
	{
		var cellNdx = (y * size) + x;
		return cells[cellNdx];
	}	

	public function fakemousemove(e:MouseEvent) 
	{
		var cursorRay : Ray = Luxe.camera.view.screen_point_to_ray( e.pos );
		cursorRay.dir.normalize();
		
		// find where the ray hits the ground plane
		var t = cursorRay.origin.y / -cursorRay.dir.y;
		cursorPos = Vector.Multiply( cursorRay.dir, t );
		cursorPos.add( cursorRay.origin );

		cursor.pos.set_xyz( cursorPos.x, cursorPos.y, cursorPos.z );	

		snapCursorPos.set_xyz( Maths.clamp( Math.floor( cursorPos.x + 0.5 ), -2.0, 2.0 ),
								0.0,
			 					Maths.clamp( Math.floor( cursorPos.z + 0.5 ), -2.0, 2.0 ));

		snowman.pos.set_xyz( snapCursorPos.x, snapCursorPos.y, snapCursorPos.z );

		// trace('gameboard mouse: ${e.x} ${e.y}');
		// trace('${cursorRay.origin.x} ${cursorRay.origin.y} ${cursorRay.origin.z}');
		// trace('${cursorPos.x} ${cursorPos.y} ${cursorPos.z}');
	}

}