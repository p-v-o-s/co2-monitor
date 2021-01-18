module enclosure() {

/* Next, we call the difference function. This specifies that we will be subtracting the second object we call from the first. We will use this to make our cube hollow. */

difference() {

/* The first object will be our main cube. to give the cube rounded edges, we call minkowski, which will trace the shape we specify around the edges. We will use a sphere, so that the hard edges of the cube will take on the shape of the sphere. */

minkowski()
{

/* Lastly, I am calling difference again here because I wanted to add a small indentation to the bottom of the cube so that it would be more comfortable to hold. Again, difference subtracts the second object from the first, so here, we see a cube; and then an offset (translated), smaller cube(); */

difference()
{
cube([60,40,15], center=true);
translate([-15,-10,-8])
cube([30,20,1.5]);
};

/* Having constructed the main box, we can now specify the size of the sphere that we will use to round the edges. */

sphere(2);
};

/* Having specified our main enclosure body with rounded edges and an indentation on the bottom, we finally hollow it out. */

cube([61.5,41.5,16], center=true);
}
}

module enclosureHoles() {

/* This section of the code constructs all of the independent holes and joins them into a uniform object. */

union() {

// USB
translate([-38,2,2])
cube([10, 9, 5]);
    
// button #1
translate([-38,-5,2])
rotate([0,90,0])
cylinder(r=2, h=30);
    
    // button #1
translate([-38,-10,2])
rotate([0,90,0])
cylinder(r=2, h=30);

// Screen
translate([-20,10,5.5])
cube([20, 8.75, 5]);

// breakout
//translate([22,-10,0])
//cube([10, 23, 8]);
    
// Volume Pot
//translate([0,-15.75,5.5])
//rotate([0,0,0])
//cylinder(r=1.25, h=5);

// Mic
translate([20,15,5.5])
rotate([0,0,0])
cylinder(r=3, h=5);

// Pushbutton #2
//translate([23.5,-12,5.5])
//rotate([0,0,0])
//cylinder(r=4.75, h=5);

// Pushbutton #3
//translate([-21.5,0,5.5])
//rotate([0,0,0])
//cylinder(r=4.75, h=5);

// Pushbutton #4
//translate([-23.5,-12,5.5])
//rotate([0,0,0])
//cylinder(r=4.75, h=5);
}
}

module texture() {
translate([0,0,9])
scale([.41,.36,.006]) surface(file="/Users/YourUsername/Path/To/Your/File/fileName.png",
center=true);
}

module concat() {

/* Difference subtracts the second object from the first */

difference() {

/* Our first object is the Union of two objects. Here, union attaches the texture to the enclosure. */

union() {
texture();
enclosure();
};

/* the semicolon signals that that is a complete object. Now the second object is the one we made from the various holes. */

enclosureHoles();
}
}

/* To render the entire design, run: */

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