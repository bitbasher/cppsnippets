/* [Hidden] */
$fs=0.5;
$fa=1.0;


translate([0,-12,0])
  color(c="red", alpha=undef) sphere(2);
translate([4,-12,0])
  color(c="red", alpha=undef) sphere(2);
translate([8,-12,0])
  color(c="red", alpha=undef) sphere(2);

color(alpha=.8) {
translate([0,-8,0])
  sphere(2);
translate([4,-8,0])
  color("violet", alpha=.2)
    sphere(2);
translate([8,-8,0])
  color(alpha=.2)sphere(2);
}

color(alpha=.5) {
translate([0,-4,0])
  color("green") sphere(2);
translate([4,-4,0])
  color("green") sphere(2);
translate([8,-4,0])
  color("green") sphere(2);
}

color([.4]) sphere(2);
translate([4,0,0])
  color([.4,.4]) sphere(2);
translate([8,0,0])
  color([.4,.4,.4]) sphere(2);

color(alpha=.5) {
translate([0,4,0]) {
  color([.4,1,1,.5]) sphere(2);
  translate([4,0,0])
    color([.4,.4,1,.5]) sphere(2);
  translate([8,0,0])
    color([.4,.4,.4,.5]) sphere(2);
  }
}
color([.6,1,1], alpha=.6) {
translate([0,8,0]) {
  color([.4,1,1,.5]) sphere(2);
  translate([4,0,0])
    color([.4,.4,1,.5]) sphere(2);
  translate([8,0,0])
    color([.4,.4,.4,.5]) sphere(2);
  }
}