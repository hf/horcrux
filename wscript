#!/bin/env python

top = '.'
out = 'build'

def options(opt):
  opt.load('compiler_cxx')

  opt.recurse('tests')

  opt.add_option('--with-tests', action='store_true', default=False, dest='horcrux_with_tests')

def configure(conf):
  conf.env.HORCRUX_WITH_TESTS = not (not conf.options.horcrux_with_tests)

  conf.load('compiler_cxx')

  conf.recurse('hx')

  if conf.env.HORCRUX_WITH_TESTS:
    conf.recurse('tests')

def build(bld):
  bld.recurse('hx')

  if bld.env.HORCRUX_WITH_TESTS:
    bld.recurse('tests')

def test(ctx):
  print
  for test in ctx.path.ant_glob(out + '/tests/test_*'):
    print('T: ' + test.name)
    ctx.exec_command(test.abspath())

def tests(ctx):
  ctx.options.horcrux_with_tests = True

  from waflib import Options
  Options.commands = ['configure', 'build', 'test'] + Options.commands
