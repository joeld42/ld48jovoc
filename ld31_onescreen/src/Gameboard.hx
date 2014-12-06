import luxe.Mesh;
import luxe.Color;
import luxe.Vector;
import luxe.Quaternion;
import luxe.Input;
import luxe.Camera;
import phoenix.Ray;
import luxe.utils.Maths;

import Tower;

class BoardCell
{	
	public var tower : Tower;

	public function new() {}
}

class Gameboard
{
	var mesh : Mesh;
	var cursor : Mesh;
	public var ghost: Mesh;

	public var cursorPos : Vector;
	public var snapCursorPos : Vector; // cursor pos snapped to a grid cell
	public var cursorBoardX : Int;
	public var cursorBoardY : Int;
	public var cursorCell : BoardCell;

	var size : Int;
	var  cells : Array<BoardCell>;

	public function new()
	{
		size = 5;
		cells = new Array<BoardCell>();

		while( cells.length < size*size )
		{
			cells.push( new BoardCell() );
		}

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

        
	}

	public function buildTower( x : Int, y : Int, tower : Tower )
	{
		var c = cell( x, y );
		c.tower = tower;
	}

	function cell( x : Int, y : Int ) : BoardCell
	{
		var cellNdx = (y * size) + x;
		return cells[cellNdx];
	}	

	public function update_ghost( placing : Bool )
	{
		if (ghost == null)
		{
			return;
		}

		if ((cursorPos.x >= -2.5) && (cursorPos.x <= 2.5) &&
			(cursorPos.z >= -2.5) && (cursorPos.z <= 2.5))
		{
	 		ghost.pos.copy_from( snapCursorPos );
	 		ghost.geometry.visible = placing;

	 		cursorBoardX = Math.floor( snapCursorPos.x + 2.51 );
	 		cursorBoardY = size - Math.floor( snapCursorPos.z + 3.51 );

	 		cursorCell = cell( cursorBoardX, cursorBoardY );

	 		// trace( 'cursor pos: ${cursorBoardX} ${cursorBoardY}');

 		}
 		else
 		{
 			ghost.geometry.visible = false;
			cursorBoardX = -1;
 			cursorBoardY = -1;
 			cursorCell = null;
 		}
	}

	public function fakemousemove(e:MouseEvent) 
	{
		var cursorRay : Ray = Luxe.camera.view.screen_point_to_ray( e.pos );
		cursorRay.dir.normalize();
		
		// find where the ray hits the ground plane
		var t = cursorRay.origin.y / -cursorRay.dir.y;
		cursorPos = Vector.Multiply( cursorRay.dir, t );
		cursorPos.add( cursorRay.origin );

		if (cursor!=null)
		{
			cursor.pos.copy_from( cursorPos );	
		}

		snapCursorPos.set_xyz( Maths.clamp( Math.floor( cursorPos.x + 0.5 ), -2.0, 2.0 ),
								0.0,
			 					Maths.clamp( Math.floor( cursorPos.z + 0.5 ), -2.0, 2.0 ));	

		// trace('gameboard mouse: ${e.x} ${e.y}');
		// trace('${cursorRay.origin.x} ${cursorRay.origin.y} ${cursorRay.origin.z}');
		// trace('${cursorPos.x} ${cursorPos.y} ${cursorPos.z}');
	}

}