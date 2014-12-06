import luxe.Mesh;
import luxe.Color;
import luxe.Vector;
import luxe.Quaternion;

class BoardCell
{

}

class Gameboard
{
	var mesh : Mesh;

	var size : Int;
	var  cells : Array<BoardCell>;

	public function new()
	{
		size = 5;
		cells = new Array<BoardCell>();
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
	}

	function cell( x : Int, y : Int ) : BoardCell
	{
		var cellNdx = (y * size) + x;
		return cells[cellNdx];
	}
}