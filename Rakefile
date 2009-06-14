# -*- mode: ruby; coding: utf-8 -*-

require 'rubygems'
require 'rake'
require 'pathname'
require 'rake/clean'
require 'rake/packagetask'
require 'rake/gempackagetask'
require 'rake/rdoctask'

dir = File.dirname(__FILE__)
$LOAD_PATH.unshift(File.join(dir, "lib"))

require 'net/drizzle/version'

spec = Gem::Specification.new do |s|
  s.name             = 'net-drizzle'
  s.version          = Net::Drizzle::Version.to_version
  s.authors          = [
    'Keiji, Yoshimi',
  ]
  s.email            = 'walf443@gmail.com'
  s.platform         = Gem::Platform::RUBY
  s.summary          = 'Ruby binding module of libdrizzle library'
  s.homepage         = 'http://walf443.github.com/net-drizzle-ruby/'

  files = FileList["{doc,ext,lib,spec}/**/*"].exclude("doc/rdoc").exclude('**/*.{bundle,o,so,log}').to_a
  files |= ['Rakefile', 'LICENSE']
  s.files            = files
  s.extensions       = ['ext/net/drizzle/extconf.rb']

  s.require_path     = 'lib'
  s.has_rdoc         = true
  s.rdoc_options     = ['--charset', 'UTF-8']
  s.extra_rdoc_files = ['README']
end

if $0 == __FILE__
  Gem::Builder.new(spec).build
  exit
end

desc 'run spec.'
task :default => %w( spec)
task :spec    => %w( compile )
task :package => %w( clean clobber rdoc )
task :clean => %w( compile_clean )

Rake::GemPackageTask.new(spec) do |pkg|
  pkg.need_tar_gz  = true
  pkg.need_tar_bz2 = true
  pkg.need_zip     = true
end

begin
  require 'spec/rake/spectask'
  Spec::Rake::SpecTask.new do |t|
    t.spec_files = FileList['spec/**/*_spec.rb']
    t.warning = false
    t.spec_opts = [ '--options', 'spec/spec.opts' ]
    # *.rb file is not exist. so skipping rcov task.
    # t.rcov = true
    # t.rcov_opts = [ '--exclude', 'spec' ]
  end
rescue LoadError => e
  warn "skipping :spec task. if you run this task, please install rspec."
end

file 'ext/net/drizzle/Makefile' do
  Dir.chdir('ext/net/drizzle/') do
    ruby ["extconf.rb", ENV["CONFIGURE_ARGS"]].join(" ")
  end
end

desc 'compile ext library'
task :compile => ['ext/net/drizzle/Makefile'] do
  Dir.chdir('ext/net/drizzle/') do
    sh "make"
  end

  Pathname('ext/net/drizzle/').children.select {|f| f.executable? }.each do |file|
    cp file, 'lib/net/drizzle/'
  end
end

desc "remove compile files"
task :compile_clean do
  if File.exist?('ext/net/drizzle/Makefile')
    rm "ext/net/drizzle/Makefile"
  end
  Pathname.glob("ext/**/*").select {|f| f.to_s !~ /\.(c|h|rb)$/ }.each do |f|
    next if f.directory?
    rm f
  end
  Pathname.glob("lib/**/*").select {|f| f.to_s !~ /\.(rb)$/ }.each do |f|
    next if f.directory?
    rm f
  end
end

Rake::RDocTask.new('rdoc') do |t|
  t.rdoc_dir = 'doc/rdoc'
  t.rdoc_files.include('README', 'lib/**/*.rb')
  t.main = 'README'
  t.title = 'Net::Drizzle Documentation'
end
