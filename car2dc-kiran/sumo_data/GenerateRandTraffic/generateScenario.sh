#!/bin/bash

set -x
set -e

if [[ $# -lt 3 ]] ; then
    echo 'usage: ./generateScenario.sh <sumoconfigname> <max_no_of_cars> <run_time>'
    exit 0
fi

scenario_name=$1
num_vehicles=$2
withTrucks=1

if [[ $# -eq 3 ]] ; then
	withTrucks=0
fi

SUMO_DIR=~/Downloads/sumo-0.25.0 #bin/sumo #/home/klingler/software/sumo-0.25.0/sumo-sommer/sumo
export SUMO_HOME="${SUMO_DIR}"
export SUMO_BINDIR="${SUMO_DIR}/bin"
export PATH="${SUMO_BINDIR}:$PATH"

street_margin=20
building_margin=15
#street_length_x=400
#street_length_y=400 # adapt this, maybe 70?
street_length_x=150
street_length_y=150 # adapt this, maybe 70?
intersections_x=5 #(change here?)
intersections_y=5 #(change here?)
lanes=3
start_time=0
end_time=$3
insert_period=0.2
seed=0
vehicle_types="vehicle-types.xml"
junction_type="allway_stop" #[traffic_light|priority|right_before_left]
#junction_type="right_before_left" # play around here?
num_vehicle_types=1000
min_distance=1000

# num_vehicles read from arguments
#num_vehicles=75

## what to change for a realistic scenario
# vehicle type distribution:  manhattan.vehicle.type.distribution_OneVehicle.xml
# 

#rm manhattan_${scenario_name}.*

#cp net/right_brefore_left.net.xml manhattan_${scenario_name}.net.xml
#cp manhattan_test/manhattan_test.net.xml .
#cp net/manhattan_test.net.xml .

${SUMO_BINDIR}/netgenerate --grid --grid.x-number ${intersections_x} --grid.y-number ${intersections_y} --grid.x-length ${street_length_x} --grid.y-length ${street_length_y} --grid.attach-length ${street_margin} -L ${lanes} -o manhattan_${scenario_name}.net.xml --default-junction-type "${junction_type}" --no-turnarounds --seed ${seed}
#--keep-edges.in-boundary "0,35,10000,545"
# --keep-edges.in-boundary "0,35,10000,745"

#--keep-edges.in-boundary "0,35,10000,545"

#if [[ $withTrucks -eq 1 ]] ; then
	#./generateVehicleTypes.py manhattan.vehicle.type.distribution.xml ${num_vehicle_types} > manhattan_${scenario_name}.add.xml
#else
	#./generateVehicleTypes.py manhattan.vehicle.type.distribution_noTrucks.xml ${num_vehicle_types} > manhattan_${scenario_name}.add.xml
#fi
./generateVehicleTypes.py manhattan.vehicle.type.distribution_OneVehicle.xml ${num_vehicle_types} > manhattan_${scenario_name}.add.xml



#####
# generate trips
####
${SUMO_HOME}/tools/randomTrips.py -n manhattan_${scenario_name}.net.xml -o manhattan_${scenario_name}.trips.xml -b ${start_time} -e ${end_time} -p ${insert_period} -s ${seed} --trip-attributes="departLane=\"best\" departSpeed=\"random\" departPos=\"random\" type=\"vTypeDist\"" --min-distance ${min_distance} --fringe-factor 1000 --intermediate 3

####
# we generate our own trips (remove this if you want to have a realistic scenario
####
# echo "<trips>" > manhattan_${scenario_name}.trips.xml
# echo "<trip id=\"0\" depart=\"0.00\" from=\"left0to0/0\" to=\"29/0toright0\" departLane=\"best\" departSpeed=\"random\" departPos=\"random\" type=\"vTypeDist\"/>" >> manhattan_${scenario_name}.trips.xml
# echo "<trip id=\"1\" depart=\"0.00\" from=\"left2to0/2\" to=\"29/2toright2\" departLane=\"best\" departSpeed=\"random\" departPos=\"random\" type=\"vTypeDist\"/>" >> manhattan_${scenario_name}.trips.xml
# echo "</trips>" >> manhattan_${scenario_name}.trips.xml

#####
# generate route
####
${SUMO_BINDIR}/duarouter -n manhattan_${scenario_name}.net.xml -d manhattan_${scenario_name}.add.xml -t manhattan_${scenario_name}.trips.xml -o manhattan_${scenario_name}.rou.xml --seed ${seed}


#${SUMO_HOME}/tools/randomTrips.py -n manhattan_${scenario_name}.net.xml -a manhattan_${scenario_name}.add.xml -o manhattan_${scenario_name}.trips.xml -b ${start_time} -e ${end_time} -p ${insert_period} -s ${seed} --min-distance ${min_distance}

#echo 'starting with duarouter\n'

#${SUMO_BINDIR}/duarouter -n manhattan_${scenario_name}.net.xml -d manhattan_${scenario_name}.add.xml -t manhattan_${scenario_name}.trips.xml -o manhattan_${scenario_name}.rou.xml --seed ${seed}
#${SUMO_HOME}/tools/assign/duaIterate.py -n manhattan_${scenario_name}.net.xml -r manhattan_${scenario_name}.rou.xml -b ${start_time} -e ${end_time} -l 3 -z


#####
# duaiterate stuff ( uncomment this 3 lines for a realistic scenario)
####
#${SUMO_HOME}/tools/assign/duaIterate.py -n manhattan_${scenario_name}.net.xml -r manhattan_${scenario_name}.rou.xml -b ${start_time} -e ${end_time} -l 3
#mv manhattan_${scenario_name}.rou.xml manhattan_${scenario_name}.rou.orig.xml
#mv manhattan_${scenario_name}_002.rou.xml manhattan_${scenario_name}.rou.xml

sed -i '/vType/d' manhattan_${scenario_name}.rou.xml
sed -i '/carFollowing/d' manhattan_${scenario_name}.rou.xml

sed -i 's/type=\"[a-zA-Z0-9]*_[a-zA-Z0-9]*\"/type=\"vTypeDist\"/g' manhattan_${scenario_name}.rou.xml
sed -i 's/type=\"[a-zA-Z0-9]*_[a-zA-Z0-9]*\"/type=\"vTypeDist\"/g' manhattan_${scenario_name}.rou.alt.xml



echo "<shapes>" > manhattan_${scenario_name}.poly.xml
count=0
y_end=$(( $intersections_y - 2 ))
x_end=$(( $intersections_x - 2 ))
for y in $(seq 0 $y_end);
do
for x in $(seq 0 $x_end);
do
  x1=$(( $x * $street_length_x + ($street_margin + $building_margin) ))
  #y1=$(( $y * $street_length_y + ($street_margin + $building_margin) -20)) # if using --keep-edges.in-boundary, then use this line
  y1=$(( $y * $street_length_y + ($street_margin + $building_margin)))

  x2=$(( $x1 ))
  #y2=$(( ($y +1) * $street_length_y + ($street_margin - $building_margin) -20)) # if using --keep-edges.in-boundary, then use this line
  y2=$(( ($y +1) * $street_length_y + ($street_margin - $building_margin)))

  x3=$(( ($x + 1) * $street_length_x + ($street_margin - $building_margin) ))
  y3=$(( $y2 ))

  x4=$(( $x3 ))
  y4=$(( $y1 ))

  count=$(( $count + 1 ))
  echo "<poly id=\"${count}\" type=\"building\" color=\"0.10,0.10,0.10\" fill=\"1\" layer=\"4\" shape=\"${x1},${y1} ${x2},${y2} ${x3},${y3} ${x4},${y4}\"/>" >> manhattan_${scenario_name}.poly.xml
done
done

echo "</shapes>" >> manhattan_${scenario_name}.poly.xml




echo "<configuration>
    <input>
        <net-file value=\"manhattan_${scenario_name}.net.xml\"/>
        <route-files value=\"manhattan_${scenario_name}.rou.xml\"/>
        <additional-files value=\"manhattan_${scenario_name}.add.xml manhattan_${scenario_name}.poly.xml\"/>
		<!-- <additional-files value=\"manhattan_${scenario_name}.poly.xml\"/> -->
    </input>
    <time>
        <begin value=\"${start_time}\"/>
        <end value=\"${end_time}\"/>
        <step-length value=\"0.1\"/>
		<start value=\"true\"/>   
    </time>
	<max-num-vehicles>${num_vehicles}</max-num-vehicles>
	<max-depart-delay>0</max-depart-delay>
</configuration>" > manhattan_${scenario_name}.sumocfg




echo "<launch>
	<copy file=\"manhattan_${scenario_name}.net.xml\" />
	<copy file=\"manhattan_${scenario_name}.rou.xml\" />
	<copy file=\"manhattan_${scenario_name}.add.xml\" />
	<copy file=\"manhattan_${scenario_name}.poly.xml\" />
	<copy file=\"manhattan_${scenario_name}.sumocfg\" type=\"config\" />
</launch>" > manhattan_${scenario_name}.launchd.xml



mkdir -p manhattan_${scenario_name}
mv manhattan_${scenario_name}.* manhattan_${scenario_name}/

