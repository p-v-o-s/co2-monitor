cavity_length=100;
cavity_width=75;
cavity_height=25;

// some dimensions used for cuts and small adjustments
bend_band=5;
slice=4;
smidge=30;

// mic
mic_radius = 5;
mic_x = 68.5; //x position of center of mic, measured from left of pcb
mic_y = 52.5; //y position of center of mic, measured from bottom edge of pcb

// button 1
button1_radius = 3.5; //mm
button1_y = 12.132; //y position of button, measured from bottom of pcb(center of its thickness)
button1_z = 5; //z position of button, measured from location of pcb bottom

// button 2
button2_radius = button1_radius;
button2_y = 22.57;
button2_z = 5;

//screen
screen_x = 20; //x position of left side of screen, measured from left side of pcb
screen_y = 32; //y position of bottom of screen, measured from bottom of pcb
screen_dx = 35; //length of screen (x dim)
screen_dy = 20; // width of screen (y dim)

// usb opening
usb_y = 44.662; //y position of usb, measured from pcb bottom
usb_dy = 10; // width of usb opening
usb_z = 15; //z position of usb, measured from pcb bottom
usb_dz = 8; // height of usb opening


difference() {
union() {
    
    //1
translate([-cavity_height,0,0])
square([cavity_height,cavity_width],false);

//2
square([cavity_length,cavity_width],false);

//3
translate([cavity_length,0,0])
square([cavity_height,cavity_width],false);

//4
translate([cavity_length+cavity_height,0,0])
square([cavity_length,cavity_width],false);

//5
translate([2*cavity_length+cavity_height,0,0])
square([cavity_height,cavity_width],false);

//6
translate([cavity_length+cavity_height,cavity_width,0])
square([cavity_length,cavity_height+bend_band],false);

//7
translate([cavity_length+cavity_height,-(cavity_height+bend_band),0])
square([cavity_length,cavity_height+bend_band],false);

//6a
translate([cavity_length+cavity_height,cavity_width+cavity_height+bend_band,0])
square([cavity_length,bend_band],false);

//7a
translate([cavity_length+cavity_height,-(cavity_height+bend_band+bend_band),0])
square([cavity_length,bend_band],false);

//8
translate([cavity_length+cavity_height,cavity_width+cavity_height+bend_band+bend_band,0])
square([cavity_length,cavity_height*.8],false);

//9
translate([cavity_length+cavity_height,-(cavity_height+bend_band+bend_band+cavity_height*.8),0])
square([cavity_length,cavity_height],false);

//3A
translate([cavity_length+slice,cavity_width,0])
square([cavity_height-1.5*slice,cavity_height],false);

//5A
translate([2*cavity_length+cavity_height+.5*slice,cavity_width,0])
square([cavity_height-2*slice,cavity_height],false);

//3B
translate([cavity_length+slice,-cavity_height,0])
square([cavity_height-1.5*slice,cavity_height],false);

//5B
translate([2*cavity_length+cavity_height+.5*slice,-cavity_height,0])
square([cavity_height-2*slice,cavity_height],false);

//8A
translate([cavity_length+cavity_height+smidge,cavity_width+cavity_height+bend_band+bend_band+cavity_height*.8,0])
square([cavity_length-2*smidge,bend_band*3],false);

//9A
translate([cavity_length+cavity_height+smidge,-(cavity_height+bend_band+bend_band+cavity_height+bend_band*2),0])
square([cavity_length-2*smidge,bend_band*3],false);


//2A
difference(){
translate([slice,cavity_width,0])
square([cavity_length-2*slice,cavity_height/2],false);

translate([slice,cavity_width,0])
polygon(points=[[0,0],[0,cavity_height],[2*slice,cavity_height],[0,0]]);
    
translate([cavity_length-slice,cavity_width,0])
polygon(points=[[0,0],[0,cavity_height],[-2*slice,cavity_height],[0,0]]);
}




//2B
difference() {
translate([slice,-cavity_height/2,0])
square([cavity_length-2*slice,cavity_height/2],false);

translate([slice,0,0])
polygon(points=[[0,0],[0,-cavity_height],[2*slice,-cavity_height],[0,0]]);
    
translate([cavity_length-slice,0,0])
polygon(points=[[0,0],[0,-cavity_height],[-2*slice,-cavity_height],[0,0]]);
    
}

//1A
difference() {
translate([-cavity_height,cavity_width,0])
square([cavity_height*.8,cavity_height],false);
    
    translate([-cavity_height,cavity_width,0])
polygon(points=[[0,0],[0,cavity_height],[slice*3,cavity_height],[0,0]]);
}

//1B
difference() {
translate([-cavity_height,-cavity_height,0])
square([cavity_height*.8,cavity_height],false);

translate([-cavity_height,-cavity_height,0])
polygon(points=[[0,0],[0,cavity_height],[slice*3,0],[0,0]]);
    
}
}


union() {
//6B
translate([cavity_length+cavity_height+smidge,cavity_width-bend_band,0])
square([cavity_length-2*smidge,bend_band],false);

//7B
translate([cavity_length+cavity_height+smidge,0,0])
square([cavity_length-2*smidge,bend_band],false);

//screen
translate([cavity_length-(screen_x+screen_dx),screen_y,0])
    square([screen_dx,screen_dy],false);

//mic
    translate([cavity_length-mic_x,mic_y,0])
    circle(r=mic_radius);
}


// button 1
button1_radius = 3.5; //mm
button1_y = 12.132; //y position of button, measured from bottom of pcb(center of its thickness)
button1_z = 5; //z position of button, measured from location of pcb bottom

translate([cavity_length+cavity_height-button1_z,button1_y,0])
circle(r=button1_radius);

// button 2
translate([cavity_length+cavity_height-button2_z,button2_y,0])
circle(r=button1_radius);

//usb 

translate([cavity_length+cavity_height-usb_z+usb_dz/2,usb_y-usb_dy/2,0])
square([usb_dz,usb_dy],false);


}

