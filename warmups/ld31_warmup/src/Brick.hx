import luxe.Component;
import luxe.Entity;
import luxe.Mesh;

class Brick extends Entity {

    public var oncerun : Bool = false;
    public var mesh : Mesh;
    public var alive: Bool = true;

    override function init() {
        trace('\tgameobject init');
    } //init

    override function onreset() {
        trace('\tgameobject reset');
    } //onreset

    override function ondestroy() {
        trace('\tgameobject destroyed');
    } //ondestroy

    override function update(dt:Float) {
        if(!oncerun){
            trace('\tgameobject first update ' + dt);
            oncerun = true;
        }
    } //update
}