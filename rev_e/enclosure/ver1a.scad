/* This module constructs the main body of the enclosure. First, we name the module: */

// box size overall
l = 95; // x dim
w = 75; // y dim
h = 42; // z dim



// button 1
button1_radius = 5; //mm
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

/* Next, we call the difference function. This specifies that we will be subtracting the second object we call from the first. We will use this to make our cube hollow. */

difference() {

/* The first object will be our main cube. to give the cube rounded edges, we call minkowski, which will trace the shape we specify around the edges. We will use a sphere, so that the hard edges of the cube will take on the shape of the sphere. */

minkowski()
{

/* Lastly, I am calling difference again here because I wanted to add a small indentation to the bottom of the cube so that it would be more comfortable to hold. Again, difference subtracts the second object from the first, so here, we see a cube; and then an offset (translated), smaller cube(); */

difference()
{
//cube([60,40,15], center=true);
//translate([-15,-10,-8])
//cube([30,20,1.5]);
    
cube([l,w,h], center=true);
//translate([-l/4,-w/4,-h/2])
//cube([l/2,w/2,h/10]);
};

/* Having constructed the main box, we can now specify the size of the sphere that we will use to round the edges. */

sphere(h/10);
};

/* Having specified our main enclosure body with rounded edges and an indentation on the bottom, we finally hollow it out. */

cube([l+1,w+1,h+1], center=true);
}
}

module enclosureHoles() {

/* This section of the code constructs all of the independent holes and joins them into a uniform object. */

union() {

// USB
    
translate([-l,usb_y-usb_dy/2,usb_z-usb_dz/2])
cube([w, usb_dy, usb_dz]);
    
    
// button #1
translate([0,button1_y,button1_z])
rotate([0,-90,0])
cylinder(r=button1_radius, h=100*l);

// button #2
translate([0,button2_y,button2_z])
rotate([0,-90,0])
cylinder(r=button2_radius, h=100*l);
    
// Screen
translate([screen_x-screen_dx/2,screen_y-screen_dy/2,h/2])
cube([screen_dx, screen_dy, 5]);


// mic
//translate([mic_x,mic_y,w/2])
translate([mic_x,mic_y,0])
cylinder(r=mic_radius, h=h*10);

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