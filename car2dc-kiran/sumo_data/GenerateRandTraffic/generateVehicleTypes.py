#!/usr/bin/env python
"""
@file    generateVehicleTypes.py
@author  Stefan Joerer
@date    2012-05-02
@version $Id$

This script generates SUMO vehicle types.

"""
import sys, datetime, random

from xml.sax import parse, handler

random.seed(0)

def getTruncatedRandomGauss(mean, sigma, trunc):
    number = random.gauss(mean, sigma)
    while (number < (mean - trunc) or number > (mean + trunc)):
        number = random.gauss(mean, sigma)
    return number


class VehicleType():
    def __init__(self):
        self.vtID = ''
        self.vtAccel = 0
        self.vtDecel = 0
        self.vtSigma = 0
        self.vtLength = 0
        self.vtMinGap = 0
        self.vtMaxSpeed = 0
        self.vtClass = ''
        self.vtGuiShape = ''
        self.vtProbability = 0.0
        self.vtAccelSigma = 0
        self.vtDecelSigma = 0
        self.vtSigmaSigma = 0
        self.vtMinGapSigma = 0
        self.vtLengthSigma = 0
        self.vtMaxSpeedSigma = 0
        self.vtCarFollowing = ''
        self.vtCarFollowingParams = []

class VehicleTypeReader(handler.ContentHandler):
    def __init__(self):
        self._flows = []
        self._vehicleTypes = []
        self._vType = VehicleType()
        self.totalProb = 0.0

    def startElement(self, name, attrs):
        if name == 'vType':
            self._vehicleTypeAttrs = attrs
            self._vType.vtID = attrs['id']
            for key in attrs.keys():
                if key == "accel":
                    self._vType.vtAccel = float(attrs['accel'])
                elif key == "decel":
                    self._vType.vtDecel = float(attrs['decel'])
                elif key == "sigma":
                    self._vType.vtSigma = float(attrs['sigma'])
                elif key == "length":
                    self._vType.vtLength = float(attrs['length'])
                elif key == "minGap":
                    self._vType.vtMinGap = float(attrs['minGap'])
                elif key == "maxSpeed":
                    self._vType.vtMaxSpeed = float(attrs['maxSpeed'])
                elif key == "vClass":
                    self._vType.vtClass = attrs['vClass']
                elif key == "guiShape":
                    self._vType.vtGuiShape = attrs['guiShape']
                elif key == "probability":
                    self._vType.vtProbability = float(attrs['probability'])
                elif key == "accelSigma":
                    self._vType.vtAccelSigma = float(attrs['accelSigma'])
                elif key == "decelSigma":
                    self._vType.vtDecelSigma = float(attrs['decelSigma'])
                elif key == "sigmaSigma":
                    self._vType.vtSigmaSigma = float(attrs['sigmaSigma'])
                elif key == "lengthSigma":
                    self._vType.vtLengthSigma = float(attrs['lengthSigma'])
                elif key == "minGapSigma":
                    self._vType.vtMinGapSigma = float(attrs['minGapSigma'])
                elif key == "maxSpeedSigma":
                    self._vType.vtMaxSpeedSigma = float(attrs['maxSpeedSigma'])
        elif name =='routes':
            print """<?xml version="1.0"?>
<!-- generated on %s by $Id$ -->""" % datetime.datetime.now()
            print '<additional>'
        elif name =='flow':
            self._flows.append(attrs)
        elif name == 'carFollowing-Krauss' or name == 'carFollowing-IDM':
            self._vType.vtCarFollowing = name
            self._vType.vtCarFollowingParams = attrs
        else:
            print '<%s' % name,
            for key in attrs.keys():
                print '%s="%s"' % (key, attrs[key]),

    def endElement(self, name):
        if name == 'vType':
            self._vehicleTypes.append(self._vType)
            self.totalProb = self.totalProb + self._vType.vtProbability
            self._vType = VehicleType()
        elif name =='routes':
            print '\n<vTypeDistribution id="vTypeDist">'
            for vt in self._vehicleTypes:
                num = int(vt.vtProbability*float(sys.argv[2]))
                for i in range(num):
                    print '<vType id="%s_%s"' % (vt.vtID,i),
                    print 'accel="%.2f"' % getTruncatedRandomGauss(vt.vtAccel, vt.vtAccelSigma, vt.vtAccelSigma),
                    print 'decel="%.2f"' % getTruncatedRandomGauss(vt.vtDecel, vt.vtDecelSigma, vt.vtDecelSigma),
                    print 'sigma="%.2f"' % getTruncatedRandomGauss(vt.vtSigma, vt.vtSigmaSigma, vt.vtSigmaSigma),
                    print 'length="%.2f"' % getTruncatedRandomGauss(vt.vtLength, vt.vtLengthSigma, vt.vtLengthSigma),
                    print 'minGap="%.2f"' % getTruncatedRandomGauss(vt.vtMinGap, vt.vtMinGapSigma, vt.vtMinGapSigma),
                    print 'maxSpeed="%.2f"' % getTruncatedRandomGauss(vt.vtMaxSpeed, vt.vtMaxSpeedSigma, vt.vtMaxSpeedSigma),
                    print 'vClass="%s"' % vt.vtClass,
                    print 'guiShape="%s"' % vt.vtGuiShape,
                    print 'probability="%s"' % (vt.vtProbability/num),
                    if vt.vtCarFollowing != '':
                        print '>'
                        print '<%s' % vt.vtCarFollowing,
                        for key in vt.vtCarFollowingParams.keys():
                            print '%s="%s"' % (key, vt.vtCarFollowingParams[key]),
                        print '/>'
                        print '</vType>'
                    else:
                        print '/>'
            print '</vTypeDistribution>'
            for f in self._flows:
                print '<flow',
                for key in f.keys():
                    print '%s="%s"' % (key, f[key]),
                print '/>'
            print '</additional>'
        elif name == 'flow' or name == 'carFollowing-Krauss' or name == 'carFollowing-IDM':
            print '',
        else:
            print '/>'


if len(sys.argv) != 3:
    print "Usage: " + sys.argv[0] + " <VehicleTypeDistribution> <NumberOfVehicleTypesToGenerate>"
    sys.exit(-1)
vtr = VehicleTypeReader()
parse(sys.argv[1], vtr)

if vtr.totalProb != 1.0 :
    print "Check VehicleType probabilities (currently sum != 1.0)!"
    sys.exit(-1)
