project(
  'dynamic_ui',
  'c',
  version: '0.1.0',
  default_options: [
    'warning_level=2',
    'c_std=c17',
  ],
)

gtk_dep  = dependency('gtk+-3.0')
json_dep = dependency('json-glib-1.0')

src_inc = include_directories('src')

subdir('assets')
subdir('config')

app_sources = files(
  'src/main.c',
  'src/app/controller.c',
  'src/app/state.c',
  'src/config/layout_store.c',
  'src/config/loader.c',
  'src/mode/manager.c',
  'src/ui/canvas.c',
  'src/ui/editor_mode.c',
  'src/ui/read_mode.c',
  'src/ui/value_item.c',
  'src/ui/window.c',
)

dynamic_ui = executable(
  'dynamic_ui',
  app_sources,
  include_directories: src_inc,
  dependencies: [
    gtk_dep,
    json_dep,
  ],
  install: true,
)

install_data(
  [
    'assets/background.png',
    'assets/app.css',
  ],
  install_dir: join_paths(get_option('datadir'), meson.project_name(), 'assets'),
)