require 'mkmf'

LIBDIR = Config::CONFIG['libdir']
INCLUDEDIR = Config::CONFIG['includedir']

$CFLAGS = '-Wall -O2'

HEADER_DIRS = [
  INCLUDEDIR,
  '/opt/local/include',
  '/usr/local/include',
  '/usr/include',
]

LIB_DIRS = [
  LIBDIR,
  '/opt/local/lib',
  '/usr/local/lib',
  '/usr/lib',
]

drizzle_dirs = dir_config('net/drizzle')
unless [nil, nil] == drizzle_dirs
  HEADER_DIRS.unshift(drizzle_dirs.first)
  LIB_DIRS.unshift(drizzle_dirs[1])
end

unless find_header('libdrizzle/drizzle_client.h', *HEADER_DIRS)
  abort "libdrizzle/drizzle_client.h is missing."
end

unless find_library('drizzle', nil, *LIB_DIRS)
  abort "drizzle is missing."
end

create_makefile('drizzle/drizzle')

