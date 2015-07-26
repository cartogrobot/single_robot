Robot Corography
----------------
This is a joint project between Aven Bross and Max Hesser-Knoll. The goal of the
project is to create a robot localization and mapping system. The system will 
utilize a single spinning range sensor (LIDAR style) to retrieve data from its
surroundings.

Data will come in as vectors from the robot's position to detected objects.
These local vectors will be converted to the world map's origin and added to the
map. We will put the map through a Hough transform to find solid edge outlines
of objects.

The feasible pose method will be used to compare new sensor data with the known
map and determine the robot's location as it moves.