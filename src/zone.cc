// Copyright 2011 Mark Cavage <mcavage@gmail.com> All rights reserved.

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#ifdef SunOS
#include <zone.h>
#include <libscf.h>
#endif

#include <node.h>
#include <v8.h>

#include <exception>
#include <vector>

#include "zutil_common.h"
#include "zone.h"

#ifdef SunOS

static v8::Handle<v8::Value> _v8Zone(zoneid_t id, const char *name) {
  v8::Local<v8::Object> zone = v8::Object::New();
  zone->Set(v8::String::New("id"), v8::Integer::New(id));
  zone->Set(v8::String::New("name"), v8::String::New(name));
  return zone;
}


v8::Handle<v8::Value> Zone::GetZone(const v8::Arguments &args) {
  v8::HandleScope scope;

  zoneid_t zoneid = -1;
  char buffer[ZONENAME_MAX] = {0};

  zoneid = getzoneid();
  if (zoneid < 0) {
    RETURN_ERRNO_EXCEPTION(errno, "getzoneid", strerror(errno));
  }
  if (getzonenamebyid(zoneid, buffer, ZONENAME_MAX) < 0) {
    RETURN_ERRNO_EXCEPTION(errno, "getzonenamebyid", strerror(errno));
  }

  return _v8Zone(zoneid, buffer);
}


v8::Handle<v8::Value> Zone::GetZoneById(const v8::Arguments &args) {
  v8::HandleScope scope;

  REQUIRE_INT_ARG(args, 0, zoneid);
  char buffer[ZONENAME_MAX] = {0};

  if (getzonenamebyid(zoneid, buffer, ZONENAME_MAX) < 0) {
    RETURN_ERRNO_EXCEPTION(errno, "getzonenamebyid", strerror(errno));
  }

  return _v8Zone(zoneid, buffer);
}


v8::Handle<v8::Value> Zone::GetZoneByName(const v8::Arguments &args) {
  v8::HandleScope scope;

  REQUIRE_STRING_ARG(args, 0, name);
  zoneid_t zoneid = -1;

  zoneid = getzoneidbyname(*name);
  if (zoneid < 0) {
    RETURN_ERRNO_EXCEPTION(errno, "getzoneidbyname", strerror(errno));
  }

  return _v8Zone(zoneid, *name);
}


v8::Handle<v8::Value> Zone::ListZones(const v8::Arguments &args) {
  char buf[ZONENAME_MAX] = {0};
  uint_t save = 0;
  uint_t nzones = 0;
  zoneid_t *zids = NULL;

 again:
  if (zone_list(NULL, &nzones) < 0) {
    RETURN_ERRNO_EXCEPTION(errno, "zone_list", strerror(errno));
  }
  save = nzones;

  zids = (zoneid_t *)calloc(nzones, sizeof(zoneid_t));
  if (zids == NULL) {
    RETURN_OOM_EXCEPTION();
  }

  if (zone_list(zids, &nzones) < 0) {
    RETURN_ERRNO_EXCEPTION(errno, "zone_list", strerror(errno));
  }

  if (nzones > save) {
    free(zids);
    goto again;
  }


  v8::Local<v8::Array> zones = v8::Array::New(nzones);
  for (uint_t i = 0; i < nzones; i++) {
    if (getzonenamebyid(zids[i], buf, ZONENAME_MAX) < 0) {
      RETURN_ERRNO_EXCEPTION(errno, "getzonenamebyid", strerror(errno));
    }
    zones->Set(v8::Integer::New(i), _v8Zone(zids[i], buf));
    memset(buf, '\0', ZONENAME_MAX);
  }

  return zones;
}


v8::Handle<v8::Value> Zone::GetZoneServiceState(const v8::Arguments &args) {
  v8::HandleScope scope;
  scf_handle_t *h = scf_handle_create(SCF_VERSION);
  scf_value_t *zone;
  scf_simple_prop_t *prop;
  const char *state_str;

  REQUIRE_STRING_ARG(args, 0, zonename);
  REQUIRE_STRING_ARG(args, 1, service);

  if ((zone = scf_value_create(h)) == NULL) {
    RETURN_EXCEPTION("Could not create scf zone value");
  }

  if (scf_value_set_astring(zone, *zonename) != SCF_SUCCESS) {
    RETURN_EXCEPTION("Could not set scf zone value");
  }

  if (scf_handle_decorate(h, "zone", zone) != SCF_SUCCESS) {
    scf_value_destroy(zone);
    RETURN_EXCEPTION("Invalid zone");
  }

  scf_value_destroy(zone);

  if (scf_handle_bind(h) == -1) {
    RETURN_EXCEPTION(scf_strerror(scf_error()));
  }

  if ((prop = scf_simple_prop_get(h, *service, SCF_PG_RESTARTER,
    SCF_PROPERTY_STATE)) == NULL) {
    RETURN_EXCEPTION(scf_strerror(scf_error()));
  }

  if ((state_str = scf_simple_prop_next_astring(prop)) == NULL) {
    scf_simple_prop_free(prop);
    RETURN_EXCEPTION(scf_strerror(scf_error()));
  }

  scf_handle_destroy(h);
  scf_simple_prop_free(prop);

  return v8::String::New(state_str);
}

#endif

void Zone::Initialize(v8::Handle<v8::Object> target) {
  v8::HandleScope scope;
#ifdef SunOS
  NODE_SET_METHOD(target, "getZone", GetZone);
  NODE_SET_METHOD(target, "getZoneById", GetZoneById);
  NODE_SET_METHOD(target, "getZoneByName", GetZoneByName);
  NODE_SET_METHOD(target, "listZones", ListZones);
  NODE_SET_METHOD(target, "getZoneServiceState", GetZoneServiceState);
#endif
}
