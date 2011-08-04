// Copyright 2011 Mark Cavage <mcavage@gmail.com> All rights reserved.
var testCase = require('nodeunit').testCase;
var uuid = require('node-uuid');

var zutil = require('../lib/zutil');

module.exports = testCase({

  setUp: function(callback) {
    callback();
  },

  tearDown: function(callback) {
    callback();
  },

  getInvalidZoneByName: function(test) {
    test.expect(1);
    test.throws(function() {
      zutil.getZoneByName(uuid());
    });
    test.done();
  },

  getInvalidZoneById: function(test) {
    test.expect(1);
    test.throws(function() {
      zutil.getZoneById(10000);
    });
    test.done();
  },

  getZone: function(test) {
    test.expect(4);
    var zone = zutil.getZone();
    test.ok(zone);
    test.ok(zone.name);
    test.equal(zone.id, 0);
    test.equal(zone.name, 'global');
    test.done();
  },

  getZoneById: function(test) {
    test.expect(4);
    var zone = zutil.getZoneById(0);
    test.ok(zone);
    test.ok(zone.name);
    test.equal(zone.id, 0);
    test.equal(zone.name, 'global');
    test.done();
  },

  getZoneByName: function(test) {
    test.expect(4);
    var zone = zutil.getZoneByName('global');
    test.ok(zone);
    test.ok(zone.name);
    test.equal(zone.id, 0);
    test.equal(zone.name, 'global');
    test.done();
  },

  listZones: function(test) {
    test.expect(4);
    var zones = zutil.listZones();
    test.ok(zones);
    test.ok(zones.length);
    test.equal(zones[0].id, 0);
    test.equal(zones[0].name, 'global');
    test.done();
  },

  getOnlineZoneServiceState: function(test) {
    var state = zutil.getZoneServiceState('adminui', 'svc:/milestone/multi-user:default');
    test.equal(state, 'online');
    test.done();
  },

  getDisabledZoneServiceStateInvalidService: function(test) {
    // Try a service that is likely to be in the disabled state.
    var state = zutil.getZoneServiceState('adminui', 'svc:/network/rexec:default');
    test.equal(state, 'disabled');
    test.done();
  },

  getZoneServiceStateInvalidZone: function(test) {
    test.throws(function () {
      zutil.getZoneServiceState('dontexist', 'svc:/milestone/multi-user:default');
    });
    test.done();
  },

  getZoneServiceStateInvalidService: function(test) {
    test.throws(function () {
      zutil.getZoneServiceState('dontexist', 'svc:/milestone/multi-user:default');
    });
    test.done();
  }
});
