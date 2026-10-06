cmake_minimum_required(VERSION 3.11)
project(PG2026-2)

# Define o padrão do C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Ativa o FetchContent
include(FetchContent)

# Configura o FetchContent para baixar e compilar GLFW automaticamente
FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG master  # Define a versão desejada da GLFW
)
FetchContent_MakeAvailable(glfw)

FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG master
)
FetchContent_MakeAvailable(glm)

# Configura o FetchContent para baixar a stb_image automaticamente
FetchContent_Declare(
  stb_image
  GIT_REPOSITORY https://github.com/nothings/stb.git
  GIT_TAG master
)
FetchContent_MakeAvailable(stb_image)

# miniaudio (música): é um único arquivo .h, então basta baixá-lo uma vez
set(MINIAUDIO_DIR ${CMAKE_BINARY_DIR}/_deps/miniaudio)
if(NOT EXISTS ${MINIAUDIO_DIR}/miniaudio.h)
  file(DOWNLOAD
    https://raw.githubusercontent.com/mackron/miniaudio/0.11.25/miniaudio.h
    ${MINIAUDIO_DIR}/miniaudio.h
    STATUS MINIAUDIO_DOWNLOAD_STATUS
  )
  list(GET MINIAUDIO_DOWNLOAD_STATUS 0 MINIAUDIO_DOWNLOAD_CODE)
  if(NOT MINIAUDIO_DOWNLOAD_CODE EQUAL 0)
    file(REMOVE ${MINIAUDIO_DIR}/miniaudio.h)
    message(FATAL_ERROR "Não foi possível baixar a miniaudio: ${MINIAUDIO_DOWNLOAD_STATUS}")
  endif()
endif()

# Adiciona as pastas de cabeçalhos
include_directories(${MINIAUDIO_DIR})
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/Common)
include_directories(${CMAKE_SOURCE_DIR}/include/glad)
include_directories(${glm_SOURCE_DIR})
include_directories(${stb_image_SOURCE_DIR})

# Lista de exercícios/projetos (um diretório para cada)
# OBS.: Não pode ter acentos ou caracteres especiais
set(EXERCISES
    Geometry-Dash
)

add_compile_options(-Wno-pragmas)

# Define as bibliotecas para cada sistema operacional
if(WIN32)
    set(OPENGL_LIBS opengl32)
    set(AUDIO_LIBS "") # a miniaudio não precisa de nenhuma biblioteca extra no Windows
elseif(APPLE)
    find_library(OpenGL_LIBRARY OpenGL)
    set(OPENGL_LIBS ${OpenGL_LIBRARY})
    set(AUDIO_LIBS "-framework CoreFoundation" "-framework CoreAudio" "-framework AudioToolbox")
else()
    find_package(OpenGL REQUIRED)
    set(OPENGL_LIBS ${OPENGL_gl_LIBRARY})
    set(AUDIO_LIBS pthread m dl)
endif()

# Caminho esperado para a GLAD
set(GLAD_C_FILE "${CMAKE_SOURCE_DIR}/Common/glad.c")

# Verifica se os arquivos da GLAD estão no lugar
if (NOT EXISTS ${GLAD_C_FILE})
    message(FATAL_ERROR "Arquivo glad.c não encontrado! 
    Baixe a GLAD manualmente em https://glad.dav1d.de/ 
    e coloque glad.h em include/glad/ e glad.c em Common/")
endif()

# Cria os executáveis (um por diretório listado em EXERCISES)
foreach(EXERCISE ${EXERCISES})
    # Extrai o nome da pasta como nome do executável
    get_filename_component(EXE_NAME ${EXERCISE} NAME)

    # Coleta todos os .cpp dentro do diretório do exercício
    file(GLOB EXE_SOURCES src/${EXERCISE}/*.cpp)

    # Cria o executável com todos os .cpp + glad.c
    add_executable(${EXE_NAME} ${EXE_SOURCES} ${GLAD_C_FILE})

    # Configura includes e bibliotecas
    target_include_directories(${EXE_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include/glad ${glm_SOURCE_DIR} ${stb_image_SOURCE_DIR})

    # Caminho absoluto da pasta de imagens, para o jogo encontrá-las
    # independente da pasta de onde o executável é chamado
    target_compile_definitions(${EXE_NAME} PRIVATE ASSETS_DIR="${CMAKE_SOURCE_DIR}/assets/")

    target_link_libraries(${EXE_NAME} glfw ${OPENGL_LIBS} ${AUDIO_LIBS} glm::glm )
endforeach()
