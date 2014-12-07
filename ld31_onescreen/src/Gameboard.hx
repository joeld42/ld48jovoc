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
	// just cache these for laziness, should
	// be a property or something
	public var gx : Int;
	public var gy : Int;

	public var tower : Tower;
	public var blocked : Bool = false;
	public var creeped : Bool = false; // is a creep on this square?
	public var creeptarg : Bool = false; // is a creep moving to this square?

	// distance to the home row (-1)
	public var homeDist : Int;

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

		// There's an extra row for pathing the creeps
		while( cells.length < size*(size+1) )
		{
			var cndx : Int = cells.length;
			var c = new BoardCell();
			c.gx = cndx % size;
			c.gy = Std.int(cndx / size);

			cells.push( c );
		}

		// initialize paths
		update_paths();

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

	public function clearBoard()
	{
		for (c in cells)
		{
			if (c.tower != null)
			{
				c.tower.mesh.destroy();
				c.tower.destroy();
				c.tower = null;
			}
			
			c.blocked = false;
			c.creeptarg = false;
			c.creeped = false;
		}

		update_paths();
	}

	public function buildTower( x : Int, y : Int, tower : Tower )
	{
		var c = cell( x, y );
		c.tower = tower;
		c.blocked = true;

		// update paths
		update_paths();
	}

	// checks that the home row is still reachable if you were to build a
	// tower here. Not a particularly cheap operation...
	public function canBuildHere( x : Int, y : Int )
	{
		var c = cell( x, y );
		// if there's already a tower here, then no..
		if ((c.tower != null) || (c.blocked))
		{
			return false;
		}

		// try marking the cell as blocked
		var result = false;

		c.blocked = true;
		update_paths();
		for (i in 0...5)
		{
			var cc = cell( i, 5);
			// trace('reachable ${i}, 5 -- ${cc.blocked}, ${cc.homeDist}');
			if (cc.homeDist < 100)
			{
				// yep, still reachable
				result = true;
				break;				
			}
		}

		// restore the path map
		c.blocked = false;
		update_paths();

		return result;
	}

	public function cell( x : Int, y : Int ) : BoardCell
	{
		if ((x<0)||(x>=5)) return null;
		if ((y<0)||(y>=6)) return null;

		var cellNdx = (y * size) + x;
		return cells[cellNdx];
	}	

	public function gridPosToWorld( x : Int, y : Int ) : Vector
	{
		var worldPos = new Vector( x - 2.0, 0, (size-y)-3.0 );
		//trace('gridPosToWorld ${x} ${y} -- ${worldPos.x} ${worldPos.z}');
		return worldPos;
	}

	public function update_paths()
	{
		for (c in cells)
		{
			// start with the home row reachable
			if ((c.gy==0) && (!c.blocked))
			{
				c.homeDist = 1;
			}
			else
			{
				c.homeDist = 9999; // uninitialized
				
				// just makes towers easier to see for debugging
				if (c.tower!=null)
				{
					c.homeDist = 8888; 
				}
			}
		}
		
		// Keep propogating distances
		var changed = false;
		do
		{
			changed = false;
			for (c in cells)
			{
				// if this cell is blocked, it will
				// always be unreachable
				if (c.blocked) continue;

				var lowCell = c.homeDist;
				for (cc in adjacentCells(c))
				{
					if ((!cc.blocked) && (cc.homeDist+1 < c.homeDist))
					{
						c.homeDist = cc.homeDist + 1;
						changed = true;
					}
				}
			}
		} while (changed);

		// printPathMap();
	}

	function printPathMap( )
	{
		// DBG print		
		trace("--- UPDATE PATHS --");
		for (j in 0...6)
		{
			var jj = 5-j;
			var rowStr = '${jj}| ';
			for (ii in 0...5)
			{
				var c = cell( ii, jj );
				var cellStr : String;
				if (c!=null)
				{
					cellStr = '${c.homeDist}';
				}
				else
				{
					cellStr = 'null';
				}

				// space pad
				while (cellStr.length < 5)
				{
					cellStr = " " + cellStr;
				}
				rowStr = rowStr + cellStr;
			}
			trace(rowStr);
		}
	}

	public function adjacentCells( cstart : BoardCell ) : Array<BoardCell>
	{
		var adj = new Array<BoardCell>();
		for (i in -1...2)
		{
			for (j in -1...2)
			{
				if ( ((i==0)||(j==0)) && (i!=j))
				{
					// cell() takes care of range checking
					var c = cell( cstart.gx + i, cstart.gy + j);
					if (c != null)
					{
						adj.push(c);
					}
				}

			}
		}
		return adj;
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
	 		//var foo = gridPosToWorld(cursorBoardX, cursorBoardY );

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