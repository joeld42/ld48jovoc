
import luxe.Color;

import mint.Control;
import mint.types.Types;
import mint.render.luxe.LuxeMintRender;
import mint.render.luxe.Convert;
import mint.layout.margins.Margins;

import SceneRender;
import Main;

class TweakPanel
{

    //var window1: mint.Window;
	var chkShadows_: mint.Checkbox;
    var progress: mint.Progress;    
    var text1: mint.TextEdit;

    var canvas : mint.Canvas;
    var windowLights_ : mint.Window;

    var scene_ : SceneRender;

    public function new( scene : SceneRender ) 
    {
		canvas = Main.canvas;
		scene_ = scene;
    }

	public function buildPanel() {

	windowLights_ = new mint.Window({
            parent: canvas,
            name: 'windowLights',
            title: 'Lighting',
            options: {
                color:new Color( 0.2, 0.2, 0.2, 0.3 ),
                color_titlebar:new Color().rgb(0x191919),
                label: { color:new Color().rgb(0x06b4fb) },
                close_button: { color:new Color().rgb(0x06b4fb) },
            },
            x:Luxe.screen.width - 300, y:0, w:300, h: Luxe.screen.height,
            w_min: 300, h_min:200,
            collapsible:true
        });

		var currY = 30;
	 	chkShadows_ = new mint.Checkbox({
            parent: windowLights_,
            name: 'chkShadows_',
            x: 10, y: currY, w: 24, h: 24,
            onchange: function(e,c) {
            	trace('Toggle shadows is ${chkShadows_.state}' );
            	scene_.showShadows_ = chkShadows_.state;
            }
        });

        new mint.Label({
            parent: windowLights_,
            name: 'lblShad',
            x:40, y:currY - 5, w:100, h:32,
            text: 'Enable shadows',
            align:left,            
            text_size: 14,
            onclick: function(e,c) {trace('hello mint! ${Luxe.time}' );}
        });
        currY += 24;

		inline function make_slider(_n,_x,_y,_w,_h,_c,_min,_max,_initial,_step:Null<Float>,_vert) : mint.Slider {

            var _s = new mint.Slider({
                parent: windowLights_, name: _n, x:_x, y:_y, w:_w, h:_h,
                options: { color_bar:new Color().rgb(_c) },
                min: _min, max: _max, step: _step, vertical:_vert, value:_initial
            });

            var _l = new mint.Label({
                parent:_s, text_size:12, x:0, y:0, w:_s.w, h:_s.h,
                align: TextAlign.center, align_vertical: TextAlign.center,
                name : _s.name+'.label', text: '${_s.value}'
            });

             _s.onchange.listen(function(_val,_) { _l.text = '$_val'; });
            return _s;
        } //make_slider
		

        // Shadow Extent -----        
		new mint.Label({
		    parent: windowLights_,
		    name: 'lblShadExt',
		    x:10, y:currY - 5, w:100, h:32,
		    text: 'Shadow Extent',
		    align:left,            
		    text_size: 14,
		    onclick: function(e,c) {trace('hello mint! ${Luxe.time}' );}
		});
		var slide = make_slider('extent', 110, currY, 180, 24, 0x9dca63, 5, 200, scene_.shadowExtent, null, false);
		slide.onchange.listen( function(_val,_) { scene_.shadowExtent = _val;  });
		currY += 32;

        // Shadow Height -----        
		new mint.Label({
		    parent: windowLights_,
		    name: 'lblShadHite',
		    x:10, y:currY - 5, w:100, h:32,
		    text: 'Shadow Height',
		    align:left,            
		    text_size: 14,
		    onclick: function(e,c) {trace('hello mint! ${Luxe.time}' );}
		});
		var slide = make_slider('hite', 110, currY, 180, 24, 0x9dca63, 5, 200, scene_.shadowHite, null, false);
		slide.onchange.listen( function(_val,_) { scene_.shadowHite = _val; });

		return;

		new mint.Label({
		    parent: canvas,
		    name: 'labelmain',
		    x:10, y:10, w:100, h:32,
		    text: 'hello mint',
		    align:left,
		    text_size: 14,
		    onclick: function(e,c) {trace('hello mint! ${Luxe.time}' );}
		});

        // check = new mint.Checkbox({
        //     parent: canvas,
        //     name: 'check1',
        //     x: 120, y: 16, w: 24, h: 24
        // });

        new mint.Checkbox({
            parent: canvas,
            name: 'check2',
            options: {
                color_node: new Color().rgb(0xf6007b),
                color_node_off: new Color().rgb(0xcecece),
                color: new Color().rgb(0xefefef),
                color_hover: new Color().rgb(0xffffff),
                color_node_hover: new Color().rgb(0xe2005a)
            },
            x: 120, y: 48, w: 24, h: 24
        });

        progress = new mint.Progress({
            parent: canvas,
            name: 'progress1',
            progress: 0.2,
            options: { color:new Color(), color_bar:new Color().rgb(0x121219) },
            x: 10, y:95 , w:128, h: 16
        });

        

        make_slider('slider1', 10, 330, 128, 24, 0x9dca63, -100, 100, 0, 10, false);
        make_slider('slider2', 10, 357, 128, 24, 0x9dca63, 0, 100, 50, 1, false);
        make_slider('slider3', 10, 385, 128, 24, 0xf6007b, null, null, null, null, false);

        make_slider('slider4', 14, 424, 32, 128, 0x9dca63, 0, 100, 20, 10, true);
        make_slider('slider5', 56, 424, 32, 128, 0x9dca63, 0, 100, 0.3, 1, true);
        make_slider('slider6', 98, 424, 32, 128, 0xf6007b, null, null, null, null, true);

        new mint.Button({
            parent: canvas,
            name: 'button1',
            x: 10, y: 52, w: 60, h: 32,
            text: 'mint',
            text_size: 14,
            options: { label: { color:new Color().rgb(0x9dca63) } },
            onclick: function(e,c) {trace('mint button! ${Luxe.time}' );}
        });

        new mint.Button({
            parent: canvas,
            name: 'button2',
            x: 76, y: 52, w: 32, h: 32,
            text: 'O',
            options: { color_hover: new Color().rgb(0xf6007b) },
            text_size: 16,
            onclick: function(e,c) {trace('mint button! ${Luxe.time}' );}
        });
    }

}