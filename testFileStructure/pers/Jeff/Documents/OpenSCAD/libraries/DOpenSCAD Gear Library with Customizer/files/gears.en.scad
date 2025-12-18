/**
    Library for involute gears: spur, helical, double helical, bevel, worm, ring, and racks.
    includes bevel, rack and pinion, and worm drive gear sets in plain, helical and
    double helical Forms
 */

// gear library version number
/* [Hidden] */
_gears_version = [2025, 05, 14, 1];

function _gears_version() =
	_gears_version;
function _gears_version_str() =
	let(g=_gears_version)
	str( g.x, g.y, g.z, ".", g[3] );

/**
Contains the modules
- rack(modul, length, height, width, pressure angle=20, helix_beta=0)
- spur_gear(modul, n_teeth_gear, width, bore_diam, helix_beta=0, hub_thick=0, hub_diameter=0, press_angle=20, optimized=true)
- rack_and_pinion(modul, length_rack, n_teeth_gear, height_rack, bore_wheel, helix_beta=0, width, hub_thick=0, hub_diameter=0, press_angle=20, make_meshing=true, optimized=true)
- ring_gear(modul, number of teeth, width, rim_width, pressure angle=20, helix_beta=0)
- herringbone_gear(modul, n_teeth_gear, width, rim width, pressure angle=20, helix_beta=0)
- planetary_gear_set(modul, n_teeth_sun, n_teeth_planet, number_planets, width, rim_width, bore, press_angle=20, helix_beta=0, assembled_tooth=true, optimized=true)
- bevel_gear(modul, n_teeth_gear, pitch_cone_angle, tooth width, bore_diam, press_angle=20, helix_beta=0)
- herringbone_bevel_gear(modul, n_teeth_gear, pitch_cone_angle, tooth_width, bore_diam, helix_beta=0, press_angle=20 )
- bevel_gear_set(modul, number of teeth_gear, number of teeth_pinion, axis angle=90, tooth width, bore_diam, helix_beta=0, press_angle = 20, make_meshing=true)
- herringbone_bevel_gear_set(modul, n_teeth_gear, n_teeth_pinion, tooth width, bore_wheel, bore_pinion, axis_angle=90, helix_beta=10, press_angle=20, make_meshing=true)
- worm(modul, number of threads, length, bore, lead_angle=10, pressure angle=20, make_meshing=true)
- worm_gear_set(modul, n_teeth_gear, n_threads, width, length, bore_worm, bore_wheel, lead_angle_phi, hub_thick, hub_diameter, press_angle=20, optimized=true, make_meshing=true)
*/

/* [gear type selection for testing] */
// can only test one gear type at a time 
Gear_type = "rack"; // [ "rack":rack, "spur_gear":spur_gear, "herringbone_gear":herringbone_gear,  "rack_and_pinion":rack_and_pinion_set, "ring_gear":ring_gear,  "herringbone_ring_gear":herringbone_ring_gear,  "planetary_gear_set":planetary_gear_set, "bevel_gear":bevel_gear, "bevel_gear_set":bevel_gear_set,  "herringbone_bevel_gear":herringbone_bevel_gear, "herringbone_bevel_gear_set":herringbone_bevel_gear_set, "worm":worm, "worm_gear_set":worm_gear_set]

/* [Options] */
/* zusammen gebaut translates to "together built" meaning that
    parts of a set of gears should be created touching.
    When set false the parts will be spaced apart to
    facilitate 3D printing
    The related module parameter is make_meshing
    there was a variable assembled = true
 */
// only for gear sets: when false lay parts out for 3D Printing
build_together = false;

/// option to perform a weighted optimization
optimized = false;  

/// option to prevent gear teeth having a helical angle.
//   all gears will have only striaght teeth
straight = false;

// option to add a hub at the center of the gear wheel
hub_wanted = true;	


/* [Basic Gear Parameters] */
/*
    These are dimensions suitable for testing.
    the small value following the controls the appearance
    of the parameter in the Customizer
 */
// module used for testing this library
Module = 1; // [0.05,0.06,0.08,0.10,0.12,0.16,0.20,0.25,0.3,0.4,0.5,0.6,0.7,0.8,0.9,1,1.25,1.5,2,2.5,3,4,5,6,8,10,12,16,20,25,32,40,50,60]

// width of the gear in mm
width = 5.0;    // [3.0:0.01:20.0]
// number of teeth on main gear
teeth = 30;     // [10:50]
// diameter of the axle hole
bore = 3.0;       // [3.0:0.01:20.0]

// diameter of optional raised hub
hub_diameter = 6;   // [3.0:0.01:20.0]
// the height of optionla raised hub
hub_thickness = 5;  // [1.0:0.20:15.0]

/* [Advanced Parameters] */
/// the most common value
// the angle at which involute gears meet
pressure_angle = 20; // [14.5, 15, 20]

helix_angle = 20;   // [-30.0:0.30:30.0]

/* [Values for multi gear sets] */
// number of teeth on the driven gear
idler_teeth = 36;   // [10:50]
// diameter of its axle hole
idler_bore = 3;     // [3.0:0.01:20.0]


/* [Values for Gear Racks] */
// length of the rack
rack_length = 50; // [10.0:0.1:100.0]
// height of the rack to the Pitch Diameter
rack_height = 4; // [2.0:0.1:15.0]

/* [Values for Worm Gears] */
// Worm lead angle, called φ (phi)
lead_angle = 4; // 0.01
// Number of thread starts
worm_starts = 1; // [1:1:4]

/** Worm bore 
  Please note: The bore parameter in Basic Gear Parameters
   is used for the worm gear. This one is for the cylidrical worm itself.
 */
// diameter of the worm's axle
worm_bore = 3;      // [3.0:0.01:20.0]
// length of the cylindrical worm
worm_length = 6;    // [10.0:0.1:100.0]

/// Test Values specific to Bevel Gear and Bevel Gear Set
/* [Values for Bevel Gears] */
bevel_angle = 45;   // [25:65]
bevel_width = 5;    // [3.0:0.1:20.0]
shaft_angle = 90;   // [60:120]

/// Test Values specific to Ring Gears. Also called Annular or Internal Gears
/* [Values for Ring Gears] */
// the witch of the side face of the ring
rim_width = 3; // [3.0:0.1:20.0]

/* [Values for Planetary Gears] */
solar_teeth = 20;   // [5:50]
planet_teeth = 10;  // [5:50]
number_of_planets=3;// [1:5]
ring_width = 5;     // [3.0:0.1:20.0]

/* [Hidden] */
finalHelixAngle = straight ? 0 : helix_angle;

/**
    the hub for the axle is always generated,
    but if not wanted is shape is given zero values 
 */
final_hub_diameter  = hub_wanted ? hub_diameter  : 0;
final_hub_thickness = hub_wanted ? hub_thickness : 0;

/**
    Unused module
module customizer_separator() {}
 */

/** smoothness factor for generated surfaces, 
    raised for this library */
$fn = 96;

/**
vocabulary - German to English

Gear Forms
zahnstange              rack
stirnrad                spur gear
pfeilrad                helical gear
zahnstange_und_rad      rack and pinion
hohlrad                 ring gear
pfeilhohlrad            herringbone gear
planetengetriebe        planetary gear
kegelrad                bevel gear
pfeilkegelrad           herringbone bevel gear
kegelradpaar            bevel gear pair
pfeilkegelradpaar       herringbone bevel gear pair
schnecke                worm
schneckenradsatz        worm gear set

Parameters
modul       module
laenge      length
hoehe       height
breite      width   // could be breadth also
randbreite  rim width
nabendicke  hub thickness 
nabendurchmesser hub diameter

/// pressure angle press_angle standard is 20 degrees
eingriffswinkel     pressure angle  
schraegungswinkel   helix angle

pitch_angle of the screw, 
    equivalent to 90° minus the helix angle.
    Positive pitch_angle = clockwise.

steigungswinkel pitch angle, symbol is γ (gamma)

zahnzahl        number of gear teeth
bohrung         bore // diameter of the gear's axle hole

Rack and Pinion Parameters
laenge stange   length rod --> rack length
hoehe stange    height rod --> rack height
bohrung rad     bore gear  --> diameter of axle hole in pinion
zahnzahl rad    number of teeth - gear --> quantity of teeth on pinion

Planetary Gear Parameters
zahnzahl sonne  number of teeth - sun
zahnzahl planet number of teeth - planets
anzahl planeten quantity of planets

Bevel Gear Parameters
teilkegelwinkel pitch cone angle --> bevel angle
zahnbreite      teeth width
zahnzahl rad    number of teeth - driving gear
zahnzahl ritzel number of teeth - pinion or driven gear
achsenwinkel    axis angle // defaults to 90 degrees

Worm Gear Parameters
gangzahl        number of threads on the cylinder worm
bohrung schnecke    bore - worm --> diameter of the axle hole in the cylidrical worm gear
bohrung rad         bore - wheel --> diameter of the axle hole in the worm gear

zusammen gebaut together built
    applies only to modules making an assembly of gears:
    name changed to make_meshing
    if true then show the component gears assembled for construction
    else show them disassembled for 3D printing

optimiert       optimized
    if the gear geometry allows, take steps to reduce material or
    or to increase surface area by adding holes in the web of the gear wheel,

There are example function and module calles for testing in
comments at the end of this file.

Author:		Dr Jörg Janssen
Date:		29. October 2018
Version:	2.3
License:	Creative Commons - Attribution, Non Commercial, Share Alike

Permitted modules according to DIN 780:
0.05 0.06 0.08 0.10 0.12 0.16
0.20 0.25 0.3  0.4  0.5  0.6
0.7  0.8  0.9  1    1.25 1.5
2    2.5  3    4    5    6
8    10   12   16   20   25
32   40   50   60

[0.05,0.06,0.08,0.10,0.12,0.16,0.20,0.25,0.3,0.4,0.5,0.6,0.7,0.8,0.9,1,1.25,1.5,2,2.5,3,4,5,6,8,10,12,16,20,25,32,40,50,60]
 */


// General Constants
pi = 3.14159;
rad = 57.29578;

/// a global parameter for the distance to leave between
///  the mating surfaces of gears created in a set
///  also call "spiel", german for play, or _play_
clearance = 0.05; // 0.01

_play_ = clearance;	// Play between mating gear teeth
OmP = 1-_play_;     // a common use for _play_
OpP = 1+_play_;     // another common use for _play_


/*	Wandelt Radian in Grad um */
function grad( angle ) = angle * rad;

/*	Wandelt Grad in Radian um */
function radian( angle ) = angle / rad;

/*	Wandelt 2D-Polarkoordinaten in kartesische um
    Format: radius, phi; phi = Winkel zur x-Achse auf xy-Ebene */
function polar_to_cart(polvect) =
    [ polvect[0]*cos(polvect[1]), polvect[0]*sin(polvect[1]) ];

/*
    Kreisevolventen-Funktion:
    Gibt die Polarkoordinaten einer Kreisevolvente aus
    r = Radius des Grundkreises
    rho = Abrollwinkel in Grad 

    Circular Involute Function:
    Returns a vector of the polar coordinates of a circular involute
        r = radius of the base circle
        rho = roll-off angle in degrees
 */
function circ_invol(r,rho) = [ r/cos(rho), grad(tan(rho)-radian(rho)) ];


/* 
    Kugelevolventen-Funktion
    Gibt den Azimutwinkel einer Kugelevolvente aus
    theta0 = Polarwinkel des Kegels, an dessen Schnittkante zur Großkugel die Evolvente abrollt
    theta = Polarwinkel, für den der Azimutwinkel der Evolvente berechnet werden soll 

    Spherical Involute Function
    Returns the azimuth angle of a spherical involute
        theta0 = polar angle of the cone at whose intersection 
                 with the large sphere the involute rolls off
        theta = polar angle for which the azimuth angle of the
                 involute is to be calculated
 */
function sphere_invol(theta0,theta) =
    1/sin(theta0)*acos(cos(theta)/cos(theta0)) -
    acos(tan(theta0)/tan(theta))
    ;

/*  Wandelt Kugelkoordinaten in kartesische um
    Format: radius, theta, phi; theta = Winkel zu z-Achse, phi = Winkel zur x-Achse auf xy-Ebene

    Converts spherical coordinates to Cartesian coordinates
        radius distance from origin to point
        theta - angle to the z-axis
        phi - angle to the x-axis on the xy plane    
 */
function sphere_to_cart(vect) = [
	vect[0]*sin(vect[1])*cos(vect[2]),  
	vect[0]*sin(vect[1])*sin(vect[2]),
	vect[0]*cos(vect[1])
    ];

/*	prüft, ob eine Zahl gerade ist
	= 1, wenn ja
	= 0, wenn die Zahl nicht gerade ist */
function istgerade(zahl) =
	(zahl == floor(zahl/2)*2) ? 1 : 0;

/*	größter gemeinsamer Teiler
	nach Euklidischem Algorithmus.
	Sortierung: a muss größer als b sein

    greatest common divisor according to the Euclidean algorithm.
        Sorting: a must be greater than b
    
    Not used in this library

function ggt(a,b) = 
	a%b == 0 ? b : ggt(b,a%b);
 */


/*	Polarfunktion mit polarwinkel und zwei variablen

    Polar function with polar angle and two variables
 */
function spiral(a, r0, phi) = a * phi + r0; 

/*	Kopiert und dreht einen Körper

    Copies and rotates a body
 */
module copy_rotate(vect, qty, distance, angle)
    {
	for(i = [0:qty-1])
        {
		translate( v=vect * distance * i )
			rotate( a= i * angle, v = [0,0,1] )
				children(0);
	    }
    }

/*  Rack (Zahnstange)
    modul = height of the tooth tip above the pitch line
    laenge = length of the rack
    hoehe = height of the rack to the pitch line
    breite = width of a tooth
    pressure angle = pressure angle,
        default value = 20° according to DIN 867.
        Should not be greater than 45°.
    ( eingriffswinkel = Eingriffswinkel, Standardwert = 20° gemäß DIN 867.
        Sollte nicht größer als 45° sein.
        )
    helix angle to the rack's transverse axis, usually called β (beta)
        default value = 0° means straight teeth
    ( schraegungswinkel = Schrägungswinkel zur Zahnstangen-Querachse;
        0° = Geradverzahnung 
        )
 */
module rack(modul, length, height, width, press_angle = 20, helix_beta = 0)
    {
	// Dimensional calculations
    modul = modul * OmP;
    c = modul / 6;                                // play, distance or backlash
    mx = modul / cos (helix_beta);                // Modulus distorted in the x-direction due to helix angle
    a = 2*mx*tan(press_angle)+c*tan(press_angle);   // Flank width
    b = pi*mx/2-2*mx*tan(press_angle);              // Tip width
    x = width*tan(helix_beta);                      // Displacement of the top surface in the x-direction due to helix angle
    nz = ceil((length+abs(2*x))/(pi*mx));           // Number of teeth

	translate([-pi*mx*(nz-1)/2-a-b/2,-modul,0])
        {
		intersection(){								// Creates a prism that fits into a cuboid geometry
			copy_rotate([1,0,0], nz, pi*mx, 0)
                {
				polyhedron(
					points=[
                        [0,-c,0],           [a,2*modul,0],          [a+b,2*modul,0], 
                        [2*a+b,-c,0],       [pi*mx,-c,0],           [pi*mx,modul-height,0],
                        [0,modul-height,0],	// Unterseite
					    [0+x,-c,width],     [a+x,2*modul,width],    [a+b+x,2*modul,width],
                        [2*a+b+x,-c,width], [pi*mx+x,-c,width],     [pi*mx+x,modul-height,width],
                        [0+x,modul-height,width]
                        ],	// top
					faces=[
                        [6,5,4,3,2,1,0],	// bottom
						[1,8,7,0],
						[9,8,1,2],
						[10,9,2,3],
						[11,10,3,4],
						[12,11,4,5],
						[13,12,5,6],
						[7,13,6,0],
						[7,8,9,10,11,12,13],// top again
					    ]
				    );
			    };
			translate([abs(x),-height+modul-0.5,-0.5])
                { // Cuboid that encompasses the volume of the rack
				cube([length,height+modul+1,width+1]);
			    }
		};
	};	
}

/*  
    Stirnrad - spur gear
    modul = Höhe des Zahnkopfes über dem Teilkreis
    zahnzahl = Anzahl der Radzähne
    breite = Zahnbreite
    bohrung = Durchmesser der Mittelbohrung
    eingriffswinkel = Eingriffswinkel, Standardwert = 20° gemäß DIN 867. Sollte nicht größer als 45° sein.
    schraegungswinkel = Schrägungswinkel zur Rotationsachse; 0° = Geradverzahnung
	optimiert = Löcher zur Material-/Gewichtsersparnis bzw. Oberflächenvergößerung erzeugen, wenn Geometrie erlaubt
 
    Spur gear
    modul = Height of the tooth tip above the pitch circle
    Number of teeth = Number of gear teeth
    Width = Tooth width
    Bore = Diameter of the center bore
    Pressure angle = Pressure angle, standard value = 20° according to DIN 867. Should not be greater than 45°.
    Helix angle = Helix angle to the rotation axis; 0° = Straight toothing
    Optimized = Create holes for material/weight savings or surface enlargement, if geometry allows

 */
module spur_gear(modul, n_teeth_gear, width, bore_diam, helix_beta=0, hub_thick=0, hub_diameter=0, press_angle=20, optimized=true)
    {
	// Dimension calculations	
	d = modul * n_teeth_gear;										// Pitch circle diameter
	r = d / 2;														// Pitch circle radius
	alpha_stirn = atan(tan(press_angle)/cos(helix_beta));           // Helix angle in the face cut
	db = d * cos(alpha_stirn);										// Base circle diameter
	rb = db / 2;													// Base circle radius
	da = (modul <1)? d + modul * 2.2 : d + modul * 2;				// Tip circle diameter according to DIN 58400 or DIN 867
	ra = da / 2;													// Tip circle radius
	c =  (n_teeth_gear <3)? 0 : modul/6;							// head play
	df = d - 2 * (modul + c);										// Root circle diameter
	rf = df / 2;													// Root circle radius
	rho_ra = acos(rb/ra);											// Maximum roll-off angle
																	// Involute begins on the base circle and ends at the tip circle
	rho_r = acos(rb/r);												// Roll-off angle at the pitch circle
																	// Involute begins on the base circle and ends at the tip circle
	pitch_gamma = 90-helix_beta;                                    // pitch angle, or gamma
    phi_r = grad(tan(rho_r)-radian(rho_r));							// Angle to the point of the involute on the pitch circle
	gamma = rad*width/(r*tan(pitch_gamma));				            // Twist angle for extrusion
	schritt = rho_ra/16;											// Involute is divided into 16 pieces
	tau = 360/n_teeth_gear;											// Pitch angle, spacing between teeth
	
	r_loch = (2*rf - bore_diam)/8;									// Radius of the holes for material/weight savings
	rm = bore_diam/2+2*r_loch;										// Distance of the hole axes from the main axis
	z_loch = floor(2*pi*rm/(3*r_loch));								// Number of holes for material/weight savings
	
	optimized = (optimized && r >= width*1.5 && d > 2*bore_diam);	// does optimization makes sense?

	// drawing
	union(){
		rotate([0,0,-phi_r-90 * OmP /n_teeth_gear])
            { // Center tooth on x-axis;
              // makes alignment with other gears easier
			linear_extrude(height = width, twist = gamma)
                {
				difference()
                    {
					union(){
						tooth_width = (180 * OmP )/n_teeth_gear+2*phi_r;
						circle(rf);										// Root circle	
						for (rot = [0:tau:360]){
							rotate (rot){								// Number of teeth times copy_rotate and rotate
								polygon(concat(							// tooth
									[[0,0]],							// Tooth segment begins and ends at the origin
									[for (rho = [0:schritt:rho_ra])		// _From_ zero degrees (base circle) _To_ the maximum involute angle (tip circle)
										polar_to_cart(circ_invol(rb,rho))], // First involute flank
									[polar_to_cart(circ_invol(rb,rho_ra))], // Point of the involute on the tip circle
									[for (rho = [rho_ra:-schritt:0])	// _From_ the maximum involute angle (tip circle) _To_ zero degrees (base circle)
										polar_to_cart( [ circ_invol(rb,rho)[0], tooth_width-circ_invol(rb,rho)[1] ] )
                                        ]
											// Second involute flank (180*(1-_play_)) instead of 180 degrees,
																		// to allow for play on the flanks
									)
								);
							}
						}
					}			
					circle(r = rm+r_loch*1.49);	// central hole for axle
				}
			}
		}
		// with material savings
		if (optimized) {
			linear_extrude(height = width){
				difference(){
						circle(r = (bore_diam+r_loch)/2);
						circle(r = bore_diam/2); // central hole for axle
					}
				}
			linear_extrude(height = (width-r_loch/2 < width*2/3) ? width*2/3 : width-r_loch/2){
				difference(){
					circle(r=rm+r_loch*1.51);
					union(){
						circle(r=(bore_diam+r_loch)/2);
						for (i = [0:1:z_loch]){
							translate(sphere_to_cart([rm,90,i*360/z_loch]))
								circle(r = r_loch);
						}
					}
				}
			}
		}
		// without material savings
		else {
			difference(){
                union(){
                    linear_extrude(height = width){
                        circle(r = rm+r_loch*1.51);
                    }
                    linear_extrude(height = width+hub_thick){
                        circle(r = hub_diameter/2);
                    }
                }
                translate([0,0,-width/2]){
                    linear_extrude(height = (width+hub_thick)*2){
                        circle(r = bore_diam/2);
                    }
                }
            }
		}
	}
}

/*
    Pfeilrad; verwendet das Modul "stirnrad"
    modul = Höhe des Zahnkopfes über dem Teilkreis
    zahnzahl = Anzahl der Radzähne
    breite = Zahnbreite
    bohrung = Durchmesser der Mittelbohrung
    eingriffswinkel = Eingriffswinkel, Standardwert = 20° gemäß DIN 867. Sollte nicht größer als 45° sein.
    schraegungswinkel = Schrägungswinkel zur Rotationsachse, Standardwert = 0° (Geradverzahnung)
	optimiert = Löcher zur Material-/Gewichtsersparnis

    herringbone gear; uses the "spur gear" module
    module = height of the tooth tip above the pitch circle
    number of teeth = number of gear teeth
    width = tooth width
    bore = diameter of the center bore
    pressure angle = pressure angle, default value = 20° according to DIN 867. Should not be greater than 45°.
    helix angle = helix angle, called beta, from the rotation axis, default value = 0° (straight teeth)
    optimized = holes for material/weight savings
 */
module herringbone_gear(modul, n_teeth_gear, width, bore_diam, helix_beta=0, hub_thick=0, hub_diameter=0, press_angle = 20, optimized=true){

	width = width/2;        // use two, half thickness, spur gears with opposing helix angle
	d = modul * n_teeth_gear;											// Teilkreisdurchmesser
	r = d / 2;														// Teilkreisradius
	c =  (n_teeth_gear <3)? 0 : modul/6;								// head play

	df = d - 2 * (modul + c);										// Fußkreisdurchmesser
	rf = df / 2;													// Fußkreisradius

	r_loch = (2*rf - bore_diam)/8;									// Radius der Löcher für Material-/Gewichtsersparnis
	rm = bore_diam/2+2*r_loch;										// Abstand der Achsen der Löcher von der Hauptachse
	z_loch = floor(2*pi*rm/(3*r_loch));								// Anzahl der Löcher für Material-/Gewichtsersparnis
	
	optimized = (optimized && r >= width*3 && d > 2*bore_diam);		// ist Optimierung sinnvoll?

	translate([0,0,width]){
		union(){
			spur_gear(modul, n_teeth_gear, width, 2*(rm+r_loch*1.49), hub_thick, hub_diameter, press_angle, helix_beta, false);		// untere Hälfte
			mirror([0,0,1]){
				spur_gear(modul, n_teeth_gear, width, 2*(rm+r_loch*1.49), hub_thick, hub_diameter, press_angle, helix_beta, false);	// obere Hälfte
			}
		}
	}
	// with material savings
	if (optimized) {
		linear_extrude(height = width*2){
			difference(){
					circle(r = (bore_diam+r_loch)/2);
					circle(r = bore_diam/2);	// Bohrung
				}
			}
		linear_extrude(height = (2*width-r_loch/2 < 1.33*width) ? 1.33*width : 2*width-r_loch/2){ //width*4/3
			difference(){
				circle(r=rm+r_loch*1.51);
				union(){
					circle(r=(bore_diam+r_loch)/2);
					for (i = [0:1:z_loch]){
						translate(sphere_to_cart([rm,90,i*360/z_loch]))
							circle(r = r_loch);
					}
				}
			}
		}
	}
	// without material savings
	else {
        difference(){
            union(){
                linear_extrude(height = width*2){
                    circle(r = rm+r_loch*1.51);
                }
                linear_extrude(height = width*2+hub_thick){
                    circle(r = hub_diameter/2);
                }
            }
            translate([0,0,-width/2]){
                linear_extrude(height = (width*2+hub_thick)*2){
                    circle(r = bore_diam/2);
                }
            }
        }
	}
}

/*
    Zahnstange und Rad
    modul = Höhe des Zahnkopfes über dem Teilkreis
    laenge_stange = Laenge der Zahnstange
    zahnzahl_rad = Anzahl der Radzähne
	hoehe_stange = Höhe der Zahnstange bis zur Wälzgeraden
    bohrung_rad = Durchmesser der Mittelbohrung des Stirnrads
	breite = Breite eines Zahns
    eingriffswinkel = Eingriffswinkel, Standardwert = 20° gemäß DIN 867. Sollte nicht größer als 45° sein.
    schraegungswinkel = Schrägungswinkel zur Rotationsachse, Standardwert = 0° (Geradverzahnung)

    Rack and pinion
    module = height of the tooth tip above the pitch circle
    length_rack = length of the rack
    number_of_tooth_wheel = number of gear teeth
    height_rack = height of the rack to the pitch line
    bore_wheel = diameter of the center bore of the spur gear
    width = width of a tooth
    pressure angle = pressure angle,
        default value = 20° according to DIN 867.
        Should not be greater than 45°.
    helix angle = helix angle to the rotation axis, 
        default value = 0° (straight gearing)

 */
module rack_and_pinion(modul, length_rack, n_teeth_gear, height_rack, bore_wheel, width, hub_thick=0, hub_diameter=0, press_angle=20, helix_beta=0, make_meshing=true, optimized=true) {

	abstand = make_meshing? modul*n_teeth_gear/2 : modul*n_teeth_gear;

	rack(modul, length_rack, height_rack, width, press_angle, -helix_beta);
	translate([0,abstand,0])
		rotate(a=360/n_teeth_gear)
			spur_gear(modul, n_teeth_gear, width, bore_wheel, hub_thick, hub_diameter, press_angle, helix_beta, optimized);
}

/*	Hohlrad
    modul = Höhe des Zahnkopfes über dem Teilkreis
    zahnzahl = Anzahl der Radzähne
    breite = Zahnbreite
	rim_width = Breite des Randes ab Fußkreis
    bohrung = Durchmesser der Mittelbohrung
    eingriffswinkel = Eingriffswinkel, Standardwert = 20° gemäß DIN 867. Sollte nicht größer als 45° sein.
    helix_beta = Schrägungswinkel zur Rotationsachse, Standardwert = 0° (Geradverzahnung)

    Ring gear
    module = height of the tooth tip above the pitch circle
    tooth_count = number of gear teeth
    width = tooth width
    rim_width = width of the rim from the root circle
    bore = diameter of the center bore
    press_angle = pressure angle, default value = 20° according to DIN 867. Should not be greater than 45°.
    helix_beta = helix angle to the rotation axis, default value = 0° (straight toothing)
 */
module ring_gear(modul, n_teeth_gear, width, rim_width, press_angle = 20, helix_beta = 0)
    {
	// Dimension calculations	
	ha = (n_teeth_gear >= 20) ? 0.02 * atan((n_teeth_gear/15)/pi) : 0.6;	// Verkürzungsfaktor Zahnkopfhöhe
	d = modul * n_teeth_gear;											// Teilkreisdurchmesser
	r = d / 2;														// Teilkreisradius
	alpha_stirn = atan(tan(press_angle)/cos(helix_beta));// Schrägungswinkel im Stirnschnitt
	db = d * cos(alpha_stirn);										// Grundkreisdurchmesser
	rb = db / 2;													// Grundkreisradius
	c = modul / 6;													// head play
	da = (modul <1)? d + (modul+c) * 2.2 : d + (modul+c) * 2;		// Kopfkreisdurchmesser
	ra = da / 2;													// Kopfkreisradius
	df = d - 2 * modul * ha;										// Fußkreisdurchmesser
	rf = df / 2;													// Fußkreisradius
	rho_ra = acos(rb/ra);											// maximaler Evolventenwinkel;
																	// Evolvente beginnt auf Grundkreis und endet an Kopfkreis
	rho_r = acos(rb/r);												// Evolventenwinkel am Teilkreis;
																	// Evolvente beginnt auf Grundkreis und endet an Kopfkreis
	phi_r = grad(tan(rho_r)-radian(rho_r));							// Winkel zum Punkt der Evolvente auf Teilkreis
	pitch_gamma = 90-helix_beta;                                    // pitch angle, or gamma
    gamma = rad*width/(r*tan(pitch_gamma));				            // twist angle for extrusion
	schritt = rho_ra/16;											// Evolvente wird in 16 Stücke geteilt
	tau = 360/n_teeth_gear;											// Pitch angle, spacing between teeth

	// Zeichnung
	rotate([0,0,-phi_r-90 * OpP/n_teeth_gear])						// Zahn auf x-Achse zentrieren;
																	// macht Ausrichtung mit anderen Rädern einfacher
	linear_extrude(height = width, twist = gamma){
		difference()
            {
			circle(r = ra + rim_width);							// Außenkreis
			union()
                {
				tooth_width = (180 * OpP)/n_teeth_gear+2*phi_r;
				circle(rf);											// Fußkreis	
				for (rot = [0:tau:360]){
					rotate (rot) {									// "Zahnzahl-mal" copy_rotaten und drehen
						polygon( concat(
							[[0,0]],
							[for (rho = [0:schritt:rho_ra])			// von null Grad (Grundkreis)
																	// bis maximaler Evolventenwinkel (Kopfkreis)
								polar_to_cart(circ_invol(rb,rho))],
							[polar_to_cart(circ_invol(rb,rho_ra))],
							[for (rho = [rho_ra:-schritt:0])		// von maximaler Evolventenwinkel (Kopfkreis)
																	// bis null Grad (Grundkreis)
								polar_to_cart([circ_invol(rb,rho)[0], tooth_width-circ_invol(rb,rho)[1]])]
																	// (180*(1+_play_)) statt 180,
																	// um Spiel an den Flanken zu erlauben
							));
					    }
				    }
			    }
		    }
	    }

	echo("Outer diameter of ring gear = ", 2*(ra + rim_width));
	
}

/*  
    Pfeil-Hohlrad; verwendet das Modul "hohlrad"
    modul = Höhe des Zahnkopfes über dem Teilkegel
    zahnzahl = Anzahl der Radzähne
    breite = Zahnbreite
    eingriffswinkel = Eingriffswinkel, Standardwert = 20° gemäß DIN 867. Sollte nicht größer als 45° sein.
    schraegungswinkel = Schrägungswinkel zur Rotationsachse, Standardwert = 0° (Geradverzahnung)

    herringbone ring gear; uses the "ring gear" module twice
    module = height of the tooth tip above the pitch cone
    number of teeth = number of gear teeth
    width = tooth width
    pressure angle = pressure angle, default value = 20° according to DIN 867. Should not be greater than 45°.
    helix angle = helix angle to the rotation axis, default value = 0° (straight toothing)

 */
module herringbone_ring_gear(modul, n_teeth_gear, width, rim_width, press_angle = 20, helix_beta = 0) {

	width = width / 2; // make the gear in two halves
	translate([0,0,width])
		union()
            { // first the upper half
		    ring_gear(modul, n_teeth_gear, width, rim_width, press_angle, helix_beta);
	        	mirror([0,0,1]) // then the lower half
			        ring_gear(modul, n_teeth_gear, width, rim_width, press_angle, helix_beta);
	        }
    }

/*	
    Planetengetriebe; verwendet die Module "pfeilrad" und "pfeilhohlrad"
    modul = Höhe des Zahnkopfes über dem Teilkegel
    zahnzahl_sonne = Anzahl der Zähne des Sonnenrads
    zahnzahl_planet = Anzahl der Zähne eines Planetenrads
    anzahl_planeten = Anzahl der Planetenräder. Wenn null, rechnet die Funktion die Mindestanzahl aus.
    breite = Zahnbreite
	randbreite = Breite des Randes ab Fußkreis
    bohrung = Durchmesser der Mittelbohrung
    eingriffswinkel = Eingriffswinkel, Standardwert = 20° gemäß DIN 867. Sollte nicht größer als 45° sein.
    schraegungswinkel = Schrägungswinkel zur Rotationsachse, Standardwert = 0° (Geradverzahnung)
	optimiert = Löcher zur Material-/Gewichtsersparnis bzw. Oberflächenvergößerung erzeugen, wenn Geometrie erlaubt
	zusammen_gebaut = Komponenten zusammengebaut für Konstruktion oder auseinander zum 3D-Druck
 
    Planetary gear; uses the "Herzrad" and "Herzholohlrad" modules.
    modul = Height of the tooth tip above the pitch cone.
    n_teeth_sun = Number of teeth on the sun gear.
    n_teeth_planet = Number of teeth on a planet gear.
    qty_planets = Number of planet gears. If zero, the function calculates the minimum number.
    width = Tooth width.
    rim_width = Width of the rim from the root circle.
    bore_diam = Diameter of the center bore.
    pressure_angle = Pressure angle, default value = 20° according to DIN 867. Should not be greater than 45°.
    helix_beta = Helix angle to the rotation axis, called beta, default value = 0° (straight gearing).
    make_meshing = when true the components will be shown assembled for construction else spread out for 3D printing.
    optimized = Create holes for material/weight savings or surface enlargement, if geometry allows.
 */
module planetary_gear_set(modul, n_teeth_sun, n_teeth_planet, qty_planets, width, rim_width, bore_diam, helix_beta=0, press_angle=20,  make_meshing=true, optimized=true)
    {
	// Dimension calculations
	d_sonne = modul*n_teeth_sun; // Teilkreisdurchmesser Sonne
	d_planet = modul*n_teeth_plane; // Teilkreisdurchmesser Planeten
	achsabstand = modul*(n_teeth_sun +  n_teeth_planet) / 2;		    // Abstand von Sonnenrad-/Hohlradachse und Planetenachse
	n_teeth_ring_gear = n_teeth_sun + 2*n_teeth_planet;				    // Anzahl der Zähne des Hohlrades
    d_hohlrad = modul*n_teeth_ring_gear;								// Teilkreisdurchmesser Hohlrad

	drehen = istgerade(n_teeth_planet);								    // Muss das Sonnenrad gedreht werden?
		
	n_max = floor(180/asin(modul*(n_teeth_planet)/(modul*(n_teeth_sun +  n_teeth_planet))));
																		// Anzahl Planetenräder: höchstens so viele, wie ohne
																		// Überlappung möglich

	// drawing sun gear
	rotate([0,0,180/n_teeth_sun*drehen])
        {
		herringbone_gear(modul, n_teeth_sun, width, bore_diam,
                helix_beta=-helix_beta, hub_thick=0, hub_diameter=0, 
                press_angle=press_angle, optimized=optimized
                );
	    }
	if (make_meshing)
        { // place the components together
        if(qty_planets==0)
            {     // first calculate then draw the planetary gears
            list = [ for( n=[2 : 1 : n_max] )
                        if( (((n_teeth_ring_gear+n_teeth_sun)/n)==floor((n_teeth_ring_gear+n_teeth_sun)/n)) )
                            n
                   ];
            qty_planets = list[0];									    // Determine the number of planetary gears
            achsabstand = modul*(n_teeth_sun + n_teeth_planet)/2;		// Distance from sun gear/ring gear axis
            for(n=[0:1:qty_planets-1]){
                translate(sphere_to_cart([achsabstand,90,360/qty_planets*n]))
					rotate([0,0,n*360*d_sonne/d_planet])
						herringbone_gear(modul, n_teeth_planet, width, bore_diam, helix_beta=helix_beta,
                                hub_thick=0, hub_diameter=0, press_angle=press_angle, optimized=optimized
                                );
                }
            }
        else {      // just draw the required number of planetary gears
            achsabstand = modul*(n_teeth_sun + n_teeth_planet)/2;		// Distance from sun gear/ring gear axis
            for(n=[0:1:qty_planets-1])
                {
                translate(sphere_to_cart([achsabstand,90,360/qty_planets*n]))
                rotate([0,0,n*360*d_sonne/(d_planet)])
                    herringbone_gear(modul, n_teeth_planet, width, bore_diam, helix_beta=helix_beta, 
                            hub_thick=0, hub_diameter=0, press_angle=press_angle, optimized=optimized
                            );
                }
		    } // end else just draw
	    }
	else {  // separate the parts for 3D printing
		planetenabstand = n_teeth_ring_gear*modul/2+rim_width+d_planet; // Distance between planets
		for(i=[-(qty_planets-1):2:(qty_planets-1)])
            {
			translate([planetenabstand, d_planet*i,0])
				herringbone_gear(modul, n_teeth_planet, width, bore_diam, hub_thick=0, hub_diameter=0, press_angle=press_angle, helix_beta=helix_beta);
		    }
	    }
    // finally, draw the ring gear
	herringbone_ring_gear(modul, n_teeth_gear, width, rim_width, press_angle, helix_beta);
}

/*  Kegelrad
    modul = Höhe des Zahnkopfes über dem Teilkegel; Angabe für die Aussenseite des Kegels
    zahnzahl = Anzahl der Radzähne
    teilkegelwinkel = (Halb)winkel des Kegels, auf dem das jeweils andere Hohlrad abrollt
    zahnbreite = Breite der Zähne von der Außenseite in Richtung Kegelspitze
    bohrung = Durchmesser der Mittelbohrung
    eingriffswinkel = Eingriffswinkel, Standardwert = 20° gemäß DIN 867. Sollte nicht größer als 45° sein.
	schraegungswinkel = Schrägungswinkel, Standardwert = 0°
 
    Bevel gear
    TODO missing the hub of the spar gear - should add 
    Module = Height of the tooth tip above the pitch cone; specification for the outer side of the cone
    Number of teeth = Number of gear teeth
    Pitch cone angle = (Half) angle of the cone on which the other ring gear rolls
    Tooth width = Width of the teeth from the outer side toward the cone tip
    Bore = Diameter of the center bore
    Pressure angle = Pressure angle, standard value = 20° according to DIN 867. Should not be greater than 45°.
    Helix angle = Helix angle, standard value = 0°
 */
module bevel_gear(modul, n_teeth_gear, pitch_cone_angle, tooth_width, bore_diam, press_angle = 20, helix_beta=0)
    {
	// Dimension calculations
	d_aussen = modul * n_teeth_gear;								// Teilkegeldurchmesser auf der Kegelgrundfläche,
																	// entspricht der Sehne im Kugelschnitt
	r_aussen = d_aussen / 2;										// Teilkegelradius auf der Kegelgrundfläche 
	rg_aussen = r_aussen/sin(pitch_cone_angle);						// Großkegelradius für Zahn-Außenseite, entspricht der Länge der Kegelflanke;
	rg_innen = rg_aussen - tooth_width;								// Großkegelradius für Zahn-Innenseite	
	r_innen = r_aussen*rg_innen/rg_aussen;
	alpha_stirn = atan(tan(press_angle)/cos(helix_beta));// Schrägungswinkel im Stirnschnitt
	delta_b = asin(cos(alpha_stirn)*sin(pitch_cone_angle));			// Grundkegelwinkel		
	da_aussen = (modul <1)? d_aussen + (modul * 2.2) * cos(pitch_cone_angle): d_aussen + modul * 2 * cos(pitch_cone_angle);
	ra_aussen = da_aussen / 2;
	delta_a = asin(ra_aussen/rg_aussen);
	c = modul / 6;													// head play
	df_aussen = d_aussen - (modul +c) * 2 * cos(pitch_cone_angle);
	rf_aussen = df_aussen / 2;
	delta_f = asin(rf_aussen/rg_aussen);
	rkf = rg_aussen*sin(delta_f);									// Radius des Kegelfußes
	hoehe_f = rg_aussen*cos(delta_f);								// Höhe des Kegels vom Fußkegel
	
	echo("Partial cone diameter on the cone base surface = ", d_aussen);
	
	// Größen für Komplementär-Kegelstumpf
	hoehe_k = (rg_aussen-tooth_width)/cos(pitch_cone_angle);			// Höhe des Komplementärkegels für richtige Zahnlänge
	rk = (rg_aussen-tooth_width)/sin(pitch_cone_angle);				// Fußradius des Komplementärkegels
	rfk = rk*hoehe_k*tan(delta_f)/(rk+hoehe_k*tan(delta_f));		// Kopfradius des Zylinders für 
																	// Komplementär-Kegelstumpf
	hoehe_fk = rk*hoehe_k/(hoehe_k*tan(delta_f)+rk);				// Hoehe des Komplementär-Kegelstumpfs

	echo("Bevel gear height = ", hoehe_f-hoehe_fk);
	
	phi_r = sphere_invol(delta_b, pitch_cone_angle);						// Winkel zum Punkt der Evolvente auf Teilkegel
		
	// Torsionswinkel gamma aus Schrägungswinkel
	gamma_g = 2*atan(tooth_width*tan(helix_beta)/(2*rg_aussen-tooth_width));
	gamma = 2*asin(rg_aussen/r_aussen*sin(gamma_g/2));
	
	schritt = (delta_a - delta_b)/16;
	tau = 360/n_teeth_gear;												// Teilungswinkel
	start = (delta_b > delta_f) ? delta_b : delta_f;
	spiegelpunkt = (180 * OmP )/n_teeth_gear+2*phi_r;

	// Zeichnung
	rotate([0,0,phi_r+90 * OmP /n_teeth_gear]){						// Zahn auf x-Achse zentrieren;
																	// macht Ausrichtung mit anderen Rädern einfacher
		translate([0,0,hoehe_f]) rotate(a=[0,180,0]){
			union(){
				translate([0,0,hoehe_f]) rotate(a=[0,180,0]){								// Kegelstumpf							
					difference(){
						linear_extrude(height=hoehe_f-hoehe_fk, scale=rfk/rkf) circle(rkf*1.001); // 1 promille Überlappung mit Zahnfuß
						translate([0,0,-1]){
							cylinder(h = hoehe_f-hoehe_fk+2, r = bore_diam/2);				// Bohrung
						}
					}	
				}
				for (rot = [0:tau:360]){
					rotate (rot) {															// "Zahnzahl-mal" copy_rotaten und drehen
						union(){
							if (delta_b > delta_f){
								// Zahnfuß
								flank_pt_low = 1*spiegelpunkt;
								flank_pt_high = sphere_invol(delta_f, start);
								polyhedron(
									points = [
										sphere_to_cart([rg_aussen, start*1.001, flank_pt_low]),	// 1 promille Überlappung mit Zahn
										sphere_to_cart([rg_innen, start*1.001, flank_pt_low+gamma]),
										sphere_to_cart([rg_innen, start*1.001, spiegelpunkt-flank_pt_low+gamma]),
										sphere_to_cart([rg_aussen, start*1.001, spiegelpunkt-flank_pt_low]),								
										sphere_to_cart([rg_aussen, delta_f, flank_pt_low]),
										sphere_to_cart([rg_innen, delta_f, flank_pt_low+gamma]),
										sphere_to_cart([rg_innen, delta_f, spiegelpunkt-flank_pt_low+gamma]),
										sphere_to_cart([rg_aussen, delta_f, spiegelpunkt-flank_pt_low])								
									    ],
									faces = [[0,1,2],[0,2,3],[0,4,1],[1,4,5],[1,5,2],[2,5,6],[2,6,3],[3,6,7],[0,3,7],[0,7,4],[4,6,5],[4,7,6]],
									convexity =1
								    );
							}
							// Zahn
							for (delta = [start:schritt:delta_a-schritt]){
								flank_pt_low = sphere_invol(delta_b, delta);
								flank_pt_high = sphere_invol(delta_b, delta+schritt);
								polyhedron(
									points = [
										sphere_to_cart([rg_aussen, delta, flank_pt_low]),
										sphere_to_cart([rg_innen, delta, flank_pt_low+gamma]),
										sphere_to_cart([rg_innen, delta, spiegelpunkt-flank_pt_low+gamma]),
										sphere_to_cart([rg_aussen, delta, spiegelpunkt-flank_pt_low]),								
										sphere_to_cart([rg_aussen, delta+schritt, flank_pt_high]),
										sphere_to_cart([rg_innen, delta+schritt, flank_pt_high+gamma]),
										sphere_to_cart([rg_innen, delta+schritt, spiegelpunkt-flank_pt_high+gamma]),
										sphere_to_cart([rg_aussen, delta+schritt, spiegelpunkt-flank_pt_high])									
									    ],
									faces = [[0,1,2],[0,2,3],[0,4,1],[1,4,5],[1,5,2],[2,5,6],[2,6,3],[3,6,7],[0,3,7],[0,7,4],[4,6,5],[4,7,6]],
									convexity =1
								    );
							}
						}
					}
				}	
			}
		}
	}
}

/*  Pfeil-Kegelrad; verwendet das Modul "kegelrad"
    modul = Höhe des Zahnkopfes über dem Teilkreis
    zahnzahl = Anzahl der Radzähne
    teilkegelwinkel, zahnbreite
    bohrung = Durchmesser der Mittelbohrung
    eingriffswinkel = Eingriffswinkel, Standardwert = 20° gemäß DIN 867. Sollte nicht größer als 45° sein.
    schraegungswinkel = Schrägungswinkel, Standardwert = 0°

    herringbone bevel gear; uses the "bevel gear" module
    module = height of the tooth tip above the pitch circle
    number of teeth = number of gear teeth
    pitch cone angle
    tooth width
    bore = diameter of the center bore
    pressure angle = pressure angle, default value = 20° according to DIN 867. Should not be greater than 45°.
    helix angle = helix angle, default value = 0°

 */
module herringbone_bevel_gear(modul, n_teeth_gear, pitch_cone_angle, tooth_width, bore_diam, helix_beta=0, press_angle = 20){

	// Dimension calculations
	
	tooth_width = tooth_width / 2;
	
	d_aussen = modul * n_teeth_gear;								// Teilkegeldurchmesser auf der Kegelgrundfläche,
																// entspricht der Sehne im Kugelschnitt
	r_aussen = d_aussen / 2;									// Teilkegelradius auf der Kegelgrundfläche 
	rg_aussen = r_aussen/sin(pitch_cone_angle);					// Großkegelradius, entspricht der Länge der Kegelflanke;
	c = modul / 6;												// head play
	df_aussen = d_aussen - (modul +c) * 2 * cos(pitch_cone_angle);
	rf_aussen = df_aussen / 2;
	delta_f = asin(rf_aussen/rg_aussen);
	hoehe_f = rg_aussen*cos(delta_f);							// Höhe des Kegels vom Fußkegel

	// Torsionswinkel gamma aus Schrägungswinkel
	gamma_g = 2*atan(tooth_width*tan(helix_beta)/(2*rg_aussen-tooth_width));
	gamma = 2*asin(rg_aussen/r_aussen*sin(gamma_g/2));
	
	echo("Partial cone diameter on the cone base surface = ", d_aussen);
	
	// Größen für Komplementär-Kegelstumpf
	hoehe_k = (rg_aussen-tooth_width)/cos(pitch_cone_angle);		// Höhe des Komplementärkegels für richtige Zahnlänge
	rk = (rg_aussen-tooth_width)/sin(pitch_cone_angle);			// Fußradius des Komplementärkegels
	rfk = rk*hoehe_k*tan(delta_f)/(rk+hoehe_k*tan(delta_f));	// Kopfradius des Zylinders für 
																// Komplementär-Kegelstumpf
	hoehe_fk = rk*hoehe_k/(hoehe_k*tan(delta_f)+rk);			// Hoehe des Komplementär-Kegelstumpfs
	
	modul_innen = modul*(1-tooth_width/rg_aussen);              // inner module ?? TODO what is this exactly?

	union(){
		bevel_gear(modul, n_teeth_gear, pitch_cone_angle, tooth_width, bore_diam, press_angle, helix_beta);		// lower half
		translate([0,0,hoehe_f-hoehe_fk])
			rotate(a=-gamma,v=[0,0,1])
				bevel_gear(modul_innen, n_teeth_gear, pitch_cone_angle, tooth_width, bore_diam, press_angle, -helix_beta);	// upper half
	}
}

/*  
    Spiral-Kegelrad; verwendet das Modul "kegelrad"
    modul = Höhe des Zahnkopfes über dem Teilkreis
    zahnzahl = Anzahl der Radzähne
    teilkegelwinkel = pitch cone angle
    hoehe = Höhe des Zahnrads
    bohrung = Durchmesser der Mittelbohrung
    eingriffswinkel = Eingriffswinkel, Standardwert = 20° gemäß DIN 867. Sollte nicht größer als 45° sein.
    schraegungswinkel = Schrägungswinkel, Standardwert = 0°

    Spiral bevel gear; uses the "bevel gear" module
    module = height of the tooth tip above the pitch circle
    number of teeth = number of gear teeth
    pitch_cone_angle = 
    height = height of the gear
    bore = diameter of the center bore
    pressure angle = pressure angle, default value = 20° according to DIN 867. Should not be greater than 45°.
    helix angle = helix angle, default value = 0°
 */
module spiral_bevel_gear(modul, n_teeth_gear, pitch_cone_angle, tooth_width, bore_diam, press_angle = 20, helix_beta=30)
    {
	schritte = 16;

	// Dimension calculations
	
	b = tooth_width / schritte;	
	d_aussen = modul * n_teeth_gear;							// Partial cone diameter on the cone base surface
																// corresponds to the chord in the spherical section
	r_aussen = d_aussen / 2;									// Partial cone radius on the cone base 
	rg_aussen = r_aussen/sin(pitch_cone_angle);					// Large cone radius, corresponds to the length of the cone flank
	rg_mitte = rg_aussen-tooth_width/2;                         // mid cone radius

	echo("Partial cone diameter on the cone base surface = ", d_aussen);

	a=tan(helix_beta)/rg_mitte;
	
	union()
        {
    	for(i=[0:1:schritte-1]){
	    	r = rg_aussen-i*b;
    		helix_beta = a*r;
	    	modul_r = modul-b*i/rg_aussen;
		    translate([0,0,b*cos(pitch_cone_angle)*i])
		    	rotate(a=-helix_beta*i,v=[0,0,1])
			    	bevel_gear(modul_r, n_teeth_gear, pitch_cone_angle, b, bore_diam, press_angle, helix_beta);	// obere Hälfte
		}
	}
}

/*	Kegelradpaar mit beliebigem Achsenwinkel; verwendet das Modul "kegelrad"
    modul = Höhe des Zahnkopfes über dem Teilkegel; Angabe für die Aussenseite des Kegels
    zahnzahl_rad = Anzahl der Radzähne am Rad
    zahnzahl_ritzel = Anzahl der Radzähne am Ritzel
	achsenwinkel = Winkel zwischen den Achsen von Rad und Ritzel
    zahnbreite = Breite der Zähne von der Außenseite in Richtung Kegelspitze
    bohrung_rad = Durchmesser der Mittelbohrung des Rads
    bohrung_ritzel = Durchmesser der Mittelbohrungen des Ritzels
    eingriffswinkel = press_angle, Standardwert = 20° gemäß DIN 867. Sollte nicht größer als 45° sein.
	schraegungswinkel = Schrägungswinkel, Standardwert = 0°
	zusammen_gebaut = Komponenten zusammengebaut für Konstruktion oder auseinander zum 3D-Druck

    Bevel gear pair with any axis angle; uses the "bevel gear" module
    module = height of the tooth tip above the pitch cone; specification for the outer side of the bevel
    gear_tooth_number = number of gear teeth on the gear
    pinion_tooth_number = number of gear teeth on the pinion
    axis_angle = angle between the axes of the gear and pinion, defaults to 90°
    tooth_width = width of the teeth from the outer side toward the cone tip
    gear_bore = diameter of the gear center bore
    pinion_bore = diameter of the pinion center bores
    pressure_angle = press_angle, default value = 20° according to DIN 867. Should not be greater than 45°.
    helix_angle = helix angle, default value = 0°
    make_meshing = components assembled for construction or disassembled for 3D printing
 */

module bevel_gear_set(modul, n_teeth_gear, n_teeth_pinion, axis_angle=90, tooth_width, bore_wheel, bore_pinion, helix_beta=0, press_angle=20, make_meshing=true)
    {
	// Dimension calculations
	r_rad = modul*n_teeth_gear/2;							// Teilkegelradius des Rads
	delta_rad = atan(sin(axis_angle)/(n_teeth_pinion/n_teeth_gear+cos(axis_angle)));	// Kegelwinkel des Rads
	delta_ritzel = atan(sin(axis_angle)/(n_teeth_gear/n_teeth_pinion+cos(axis_angle)));// Kegelwingel des Ritzels
	rg = r_rad/sin(delta_rad);								// Radius der Großkugel
	c = modul / 6;											// head play
	df_ritzel = pi*rg*delta_ritzel/90 - 2 * (modul + c);	// Fußkegeldurchmesser auf der Großkugel 
	rf_ritzel = df_ritzel / 2;								// Fußkegelradius auf der Großkugel
	delta_f_ritzel = rf_ritzel/(pi*rg) * 180;				// Kopfkegelwinkel
	rkf_ritzel = rg*sin(delta_f_ritzel);					// Radius des Kegelfußes
	hoehe_f_ritzel = rg*cos(delta_f_ritzel);				// Höhe des Kegels vom Fußkegel
	
	echo("Cone angle gear = ",   delta_rad   );
	echo("Cone angle pinion = ", delta_ritzel);
 
	df_rad = pi*rg*delta_rad/90 - 2 * (modul + c);			// Fußkegeldurchmesser auf der Großkugel 
	rf_rad = df_rad / 2;									// Fußkegelradius auf der Großkugel
	delta_f_rad = rf_rad/(pi*rg) * 180;						// Kopfkegelwinkel
	rkf_rad = rg*sin(delta_f_rad);							// Radius des Kegelfußes
	hoehe_f_rad = rg*cos(delta_f_rad);						// Höhe des Kegels vom Fußkegel

	echo("height of Gear   = ", hoehe_f_rad);
	echo("Height of Pinion = ", hoehe_f_ritzel);
	
	drehen = istgerade(n_teeth_pinion);
	
	// Drawing
	// gear
	rotate([0,0,180 * OmP /n_teeth_gear*drehen])
		bevel_gear(modul, n_teeth_gear, delta_rad, tooth_width, bore_wheel, helix_beta, press_angle );
	
	// pinion
	if (make_meshing)
		translate([-hoehe_f_ritzel*cos(90-axis_angle),0,hoehe_f_rad-hoehe_f_ritzel*sin(90-axis_angle)])
			rotate([0,axis_angle,0])
				bevel_gear(modul, n_teeth_pinion, delta_ritzel, tooth_width, bore_pinion, -helix_beta, press_angle );
	else
		translate([rkf_ritzel*2+modul+rkf_rad,0,0])
			bevel_gear(modul, n_teeth_pinion, delta_ritzel, tooth_width, bore_pinion, -helix_beta, press_angle );
 }

/*	Pfeil-Kegelradpaar mit beliebigem Achsenwinkel; verwendet das Modul "pfeilkegelrad"
    modul = Höhe des Zahnkopfes über dem Teilkegel; Angabe für die Aussenseite des Kegels
    zahnzahl_rad = Anzahl der Radzähne am Rad
    zahnzahl_ritzel = Anzahl der Radzähne am Ritzel
	achsenwinkel = Winkel zwischen den Achsen von Rad und Ritzel
    zahnbreite = Breite der Zähne von der Außenseite in Richtung Kegelspitze
    bohrung_rad = Durchmesser der Mittelbohrung des Rads
    bohrung_ritzel = Durchmesser der Mittelbohrungen des Ritzels
    eingriffswinkel = Eingriffswinkel, Standardwert = 20° gemäß DIN 867. Sollte nicht größer als 45° sein.
    schraegungswinkel = Schrägungswinkel, Standardwert = 0°
	zusammen_gebaut = Komponenten zusammengebaut für Konstruktion oder auseinander zum 3D-Druck

    herringbone bevel gear pair with any axis angle; uses the "arrowhead bevel gear" module

    module = height of the tooth tip above the pitch cone; specification for the outer side of the bevel
    gear_tooth_number = number of gear teeth on the gear
    pinion_tooth_number = number of gear teeth on the pinion
    axis_angle = angle between the axes of the gear and pinion
    tooth_width = width of the teeth from the outer side toward the cone tip
    gear_bore = diameter of the gear center bore
    pinion_bore = diameter of the pinion center bores
    pressure_angle = pressure angle, default value = 20° according to DIN 867. Should not be greater than 45°.
    helix_beta = helix angle, default value = 0°
    make_meshing = components assembled for construction or disassembled for 3D printing

 */
module herringbone_bevel_gear_set(modul, n_teeth_gear, n_teeth_pinion, tooth_width, bore_wheel, bore_pinion, helix_beta=10, axis_angle=90, press_angle = 20, make_meshing=true )
    {
	r_rad = modul*n_teeth_gear/2;							// Gear pitch cone radius
	delta_rad = atan(sin(axis_angle)/
        (n_teeth_pinion/n_teeth_gear+cos(axis_angle)));     // Gear cone angle
	delta_ritzel = atan(sin(axis_angle)/
        (n_teeth_gear/n_teeth_pinion+cos(axis_angle)));     // Pinion cone angle
	rg = r_rad/sin(delta_rad);								// Radius of the large sphere
	c = modul / 6;											// head play
	df_ritzel = pi*rg*delta_ritzel/90 - 2 * (modul + c);	// Root cone diameter on the large sphere 
	rf_ritzel = df_ritzel / 2;								// Root cone radius on the large sphere
	delta_f_ritzel = rf_ritzel/(pi*rg) * 180;				// Tip cone angle
	rkf_ritzel = rg*sin(delta_f_ritzel);					// Radius of the cone root
	hoehe_f_ritzel = rg*cos(delta_f_ritzel);				// Height of the cone from the root cone
	
	echo("Cone angle gear   = ", delta_rad   );
	echo("Cone angle pinion = ", delta_ritzel);
 
	df_rad = pi*rg*delta_rad/90 - 2 * (modul + c);			// Root cone diameter on the large sphere 
	rf_rad = df_rad / 2;									// Root cone radius on the large sphere
	delta_f_rad = rf_rad/(pi*rg) * 180;						// Tip cone angle
	rkf_rad = rg*sin(delta_f_rad);							// Radius of the cone root
	hoehe_f_rad = rg*cos(delta_f_rad);						// Height of the cone from the base cone

	echo("height gear   = ", hoehe_f_rad   );
	echo("height pinion = ", hoehe_f_ritzel);
	
	drehen = istgerade(n_teeth_pinion);
	
	// Draw the Main Gear
	rotate([0,0,180 * OmP /n_teeth_gear*drehen])
		herringbone_bevel_gear(modul, n_teeth_gear, delta_rad, tooth_width, bore_wheel, helix_beta, press_angle);
	
	// Draw the Pinion
	if (make_meshing)
		translate([-hoehe_f_ritzel*cos(90-axis_angle),0,hoehe_f_rad-hoehe_f_ritzel*sin(90-axis_angle)])
			rotate([0,axis_angle,0])
				herringbone_bevel_gear(modul, n_teeth_pinion, delta_ritzel, tooth_width, bore_pinion, -helix_beta, press_angle);
	else
		translate([rkf_ritzel*2+modul+rkf_rad,0,0])
			herringbone_bevel_gear(modul, n_teeth_pinion, delta_ritzel, tooth_width, bore_pinion, -helix_beta, press_angle);

}

/*
    Berechnet eine Schnecke / archimedische Schraube.
    modul = Höhe des Schneckenkopfes über dem Teilzylinder
    gangzahl = Anzahl der Gänge (Zähne) der Schnecke
    laenge = Länge der Schnecke
    bohrung = Durchmesser der Mittelbohrung
    eingriffswinkel = Eingriffswinkel, Standardwert = 20° gemäß DIN 867. Sollte nicht größer als 45° sein.
    steigungswinkel = Steigungswinkel der Schnecke, entspricht 90° minus Schrägungswinkel. Positiver Steigungswinkel = rechtsdrehend.
    zusammen_gebaut = Komponenten zusammengebaut für Konstruktion oder auseinander zum 3D-Druck

    Calculates a cylindrical worm gear as an Archimedes screw.
     The threads of the screw are equivalent to the teeth of a spur gear

    module = height of the screw head above the partial cylinder radius
    n_threads = number of threads on the screw
    length = length of the screw
    bore_diam = diameter of the hole for the axle
    pressure angle = pressure angle, default value = 20° according to DIN 867. Should not be greater than 45°.
    lead_angle_phi = lead angle of the screw, corresponds to 90° minus the helix angle. Positive helix angle = clockwise.
    make_meshing = when true the components will be shown assembled for construction else spread out for 3D printing

number of threads = number of threads of the worm, equiv to teeth for a gear
 */
module worm(modul, n_threads, length, bore_diam, lead_angle_phi, press_angle=20, make_meshing=true)
    {
	// Dimension calculations
	c = modul / 6;											// head play
	r = modul*n_threads/(2*sin(lead_angle_phi));			// Partial cylinder radius
	rf = r - modul - c;										// Base cylinder radius
	a = modul*n_threads/(90*tan(press_angle));				// Spiral parameters
	tau_max = 180/n_threads*tan(press_angle);				// Angle from base to head in the normal direction
	gamma = -rad*length/((rf+modul+c)*tan(lead_angle_phi));	// Torsion angle for extrusion
	
	schritt = tau_max/16;
	
    // Drawing: extrude a surface with a twist, enclosed by two Archimedean spirals
	if (make_meshing) {
		rotate([0,0,tau_max])
            {
			linear_extrude(height = length, center = false, convexity = 10, twist = gamma){
				difference(){
					union(){
						for(i=[0:1:n_threads-1])
                            {
							polygon(
								concat(							
									[[0,0]],    // start at origin
									[for (tau = [0:schritt:tau_max])  // rising tooth flank
										polar_to_cart( [ spiral(a, rf, tau), tau+i*(360/n_threads) ] )
                                        ],
									[for (tau = [tau_max:schritt:180/n_threads]) // tip of tooth
										polar_to_cart([spiral(a, rf, tau_max), tau+i*(360/n_threads)])],
									[for (tau = [180/n_threads:schritt:(180/n_threads+tau_max)])  // descending tooth flank
										polar_to_cart([spiral(a, rf, 180/n_threads+tau_max-tau), tau+i*(360/n_threads) ] ) 
                                        ]
								    )
							    );
						    } // end for(threads)
						circle(rf);
					}
					circle(bore_diam/2); // Center hole
				}
			}
		}
	}
	else { // make two half objects for 3D printing
		difference()
            {
			union(){
				translate([1,r*1.5,0])
                    {
					rotate([90,0,90])
						worm(modul, n_threads, length, bore_diam, lead_angle_phi, press_angle, make_meshing=true);
				    }
				translate([length+1,-r*1.5,0])
                    {
					rotate([90,0,-90])
						worm(modul, n_threads, length, bore_diam, lead_angle_phi, press_angle, make_meshing=true);
					}
				}
			translate([length/2+1,0,-(r+modul+1)/2])
                {
				cube([length+2,3*r+2*(r+modul+1),r+modul+1], center = true);
				}
		    }
	    }
    }

/*
Berechnet einen Schneckenradsatz. Das Schneckenrad ist ein gewöhnliches Stirnrad ohne Globoidgeometrie.
modul = Höhe des Schneckenkopfes über dem Teilzylinder bzw. des Zahnkopfes über dem Teilkreis
zahnzahl = Anzahl der Radzähne
gangzahl = Anzahl der Gänge (Zähne) der Schnecke
breite = Zahnbreite
laenge = Länge der Schnecke
bohrung_schnecke = Durchmesser der Mittelbohrung der Schnecke
bohrung_rad = Durchmesser der Mittelbohrung des Stirnrads
eingriffswinkel = Eingriffswinkel, Standardwert = 20° gemäß DIN 867. Sollte nicht größer als 45° sein.
steigungswinkel = Steigungswinkel der Schnecke, entspricht 90°-Schrägungswinkel. Positiver Steigungswinkel = rechtsdrehend.
optimiert = Löcher zur Material-/Gewichtsersparnis
zusammen_gebaut =  Komponenten zusammengebaut für Konstruktion oder auseinander zum 3D-Druck

Calculates a worm gear set. The worm gear is a conventional spur gear without globoid geometry.
module = height of the worm head above the pitch cylinder or the tooth head above the pitch circle
tooth count = number of gear teeth
start count = number of threads (teeth) of the worm
width = tooth width
length = length of the worm
bore_worm = diameter of the center bore of the worm
bore_wheel = diameter of the center bore of the spur gear
pressure angle = pressure angle, default value = 20° according to DIN 867. Should not be greater than 45°.
helix angle = lead angle of the worm, corresponds to a 90° helix angle. Positive helix angle = clockwise.
optimized = holes for material/weight savings
make_meshing = when true the components will be shown assembled for construction else spread out for 3D printing
 */
module worm_gear_set(modul, n_teeth_gear, n_threads, width, length, bore_worm, bore_wheel, lead_angle_phi, hub_thick=0, hub_diameter=0, press_angle=20, optimized=true, make_meshing=true)
    {
	c = modul / 6;											// head play
	r_schnecke = modul*n_threads/(2*sin(lead_angle_phi));		// Partial cylinder radius screw
	r_rad = modul*n_teeth_gear/2;							// Pitch cone radius spur gear
	rf_schnecke = r_schnecke - modul - c;					// Foot cylinder radius
	gamma = -90*width*sin(lead_angle_phi)/(pi*r_rad);			// Rotation angle spur gear
	zahnabstand = modul*pi/cos(lead_angle_phi);				// Tooth spacing in transverse section
	x = istgerade(n_threads)? 0.5 : 1;

	if (make_meshing)
        {
		translate([r_schnecke,(ceil(length/(2*zahnabstand))-x)*zahnabstand,0])
			rotate([90,180/n_threads,0])
				worm(modul, n_threads, length, bore_worm, press_angle, lead_angle_phi, make_meshing);

		translate([-r_rad,0,-width/2])
			rotate([0,0,gamma])
				spur_gear(modul, n_teeth_gear, width, bore_wheel, hub_thick, hub_diameter, press_angle, -lead_angle_phi, optimized);
	    }
	else {	
		worm(modul, n_threads, length, bore_worm, lead_angle_phi, press_angle, make_meshing);
		translate([-2*r_rad,0,0])
			spur_gear(modul, n_teeth_gear, width, bore_wheel, hub_thick, hub_diameter, press_angle, -lead_angle_phi, optimized);
	    }
}



if(Gear_type == "rack") {
    rack(modul=Module, length=rack_length, height=rack_height, width=width, 
    press_angle=pressure_angle, helix_beta=finalHelixAngle);

} else if(Gear_type == "spur_gear" ) {
    spur_gear(modul=Module, n_teeth_gear=teeth, width=width, 
        bore_diam=bore, helix_beta=finalHelixAngle,
        hub_diameter=final_hub_diameter, hub_thick=final_hub_thickness,
        press_angle=pressure_angle, optimized=optimized);

} else if(Gear_type == "herringbone_gear" ) {
    herringbone_gear(modul=Module, n_teeth_gear=teeth,
    width=width, bore_diam=bore, helix_beta=finalHelixAngle,
    hub_thick=final_hub_thickness, hub_diameter=final_hub_diameter, 
    press_angle=pressure_angle, optimized=optimized);

} else if(Gear_type == "rack_and_pinion" ) {
    rack_and_pinion(modul=Module, length_rack=rack_length, 
        n_teeth_gear=teeth, height_rack=rack_height,
        bore_wheel=bore, helix_beta=finalHelixAngle, 
        width=width,
        hub_thick=final_hub_thickness, hub_diameter=final_hub_diameter, 
        press_angle=pressure_angle, make_meshing=build_together, optimized=optimized
        );

} else if(  Gear_type == "annular_spur_gear"    || 
            Gear_type == "internal_spur_gear"   ||
            Gear_type == "ring_gear") {
    ring_gear(modul=Module, n_teeth_gear=teeth, width=width, rim_width=rim_width, press_angle=pressure_angle, helix_beta=finalHelixAngle);

} else if(  Gear_type == "annular_herringbone_gear"     ||
            Gear_type == "internal_herringbone_gear"    ||
            Gear_type == "herringbone_ring_gear") {
    herringbone_ring_gear (modul=Module, n_teeth_gear=teeth, width=width, rim_width=rim_width, press_angle=pressure_angle, helix_beta=finalHelixAngle);

} else if(Gear_type == "planetary_gear_set" ) {
    planetary_gear_set(modul=Module, n_teeth_sun=solar_teeth, n_teeth_planet=planet_teeth,
            qty_planets=number_of_planets, width=width, rim_width=rim_width,
            bore=bore, helix_beta=finalHelixAngle, press_angle=pressure_angle, 
            make_meshing=true, optimized=optimized);

} else if(Gear_type == "bevel_gear" ) {
    bevel_gear(modul=Module, n_teeth_gear=teeth,  pitch_cone_angle=bevel_angle, 
            tooth_width=bevel_width, bore_diam=bore,
            press_angle=pressure_angle, helix_beta=finalHelixAngle
            );

} else if(Gear_type == "herringbone_bevel_gear" ) {
    herringbone_bevel_gear(modul=Module, n_teeth_gear=teeth, pitch_cone_angle=bevel_angle, 
            tooth_width=bevel_width, bore_diam=bore, press_angle=pressure_angle, 
            helix_beta=finalHelixAngle
            );

} else if(Gear_type == "bevel_gear_set" ) {
    bevel_gear_set(modul=Module, n_teeth_gear=idler_teeth, bore_wheel=teeth, axis_angle=shaft_angle, tooth_width=bevel_width, bore_wheel=idler_bore, bore_pinion=bore, press_angle=pressure_angle, helix_beta=finalHelixAngle, make_meshing=build_together);

} else if(Gear_type == "herringbone_bevel_gear_set" ) {
    herringbone_bevel_gear_set(modul=Module, n_teeth_gear=idler_teeth, bore_wheel=teeth, axis_angle=shaft_angle, tooth_width=bevel_width, bore_wheel=idler_bore, bore_pinion=bore, press_angle=pressure_angle, helix_beta=finalHelixAngle, make_meshing=build_together);

} else if(Gear_type == "worm" ) {
    worm(modul=Module, n_threads=worm_starts, length=worm_length, 
            bore_diam=worm_bore, lead_angle_phi=lead_angle, 
            press_angle=pressure_angle, make_meshing=build_together
            );

} else if(Gear_type == "worm_gear_set" ) {
    worm_gear_set(modul=Module, n_teeth_gear=teeth, n_threads=worm_starts, 
        width=width, length=worm_length, bore_worm=worm_bore, bore_wheel=bore,
        lead_angle_phi=lead_angle,
        hub_diameter=final_hub_diameter, hub_thick=final_hub_thickness, 
        press_angle=pressure_angle, optimized=optimized, make_meshing=build_together
        );
}

