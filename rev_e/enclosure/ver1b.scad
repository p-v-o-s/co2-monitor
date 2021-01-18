/* This module constructs the main body of the enclosure. First, we name the module: */

//pcb dimensions (mm)
pcb_l = 92; // (x dim, edges of PCB)
pcb_w = 69.5; // (y dim, edges of PCB)
pcb_h = 20; // (z dim, from bottom of PCB to top with placed components / headers / etc)

buffer = 10; // empty space in each dim
wall_thickness = 1; //

// box size overall
l = pcb_l+buffer; // x dim
w = pcb_w+buffer; // y dim
h = pcb_h+buffer; // z dim

wall=1; // wall thickness

//upper box height (pcb up)
h_up = 25;

//lower box height
h_down = 15;

// button 1
button1_radius = 3.5; //mm
button1_y = -12.09;
button1_z = 5;

// button 2
button2_radius = button1_radius;
button2_y = -22.53;
button2_z = button1_z;

// mic
mic_radius = 7;
mic_x = 22.5;
mic_y = 17;

// usb opening
usb_y = 9.98;
usb_dy = 10;
usb_z = 15;
usb_dz = 10;

//screen
screen_x = -7; //x position of center of screen
screen_y = 10; //y position of center of screen
screen_dx = 35; //length of screen (x dim)
screen_dy = 20; //

module enclosure() {

difference() {

minkowski()
{

difference()
{
translate([0,0,0])
cube([l,w,h], center=true);
};

sphere(h/10);
};

cube([l+wall_thickness,w+wall_thickness,h+wall_thickness], center=true);
}
}

module enclosureHoles() {

union() {

// USB
translate([-l/2-10,usb_y-usb_dy/2,usb_z-usb_dz/2])
cube([10*wall, usb_dy, usb_dz]);
    
    
// button #1
translate([-l/2,button1_y,button1_z])
rotate([0,-90,0])
cylinder(r=button1_radius, h=10*wall);

// button #2
translate([-l/2,button2_y,button2_z])
rotate([0,-90,0])
cylinder(r=button2_radius, h=10*wall);
    
// Screen
translate([screen_x-screen_dx/2,screen_y-screen_dy/2,h/2])
cube([screen_dx, screen_dy,10*wall]);


// mic
translate([mic_x,mic_y,h/2-10*wall/2])
cylinder(r=mic_radius, h=20*wall);

}
}

module concat() {

difference() {
enclosure();
enclosureHoles();
}
}

concat();

/* To actually print, we’ll need to render it in two separate halves which we will attach later. So, comment out the above concat() command and instead run the below code to render the top only */

difference() {
concat();
translate([0,0,-8.5])
cube([65,44,2], center=true);
}

/* then, comment the above out and run the following code to render the bottom only */

difference() {
concat();
translate([0,0,2])
cube([65,44,16], center=true);
}