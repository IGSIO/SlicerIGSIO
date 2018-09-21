
#************************************************************************
#*************************** vtkEncodeUncompressedSequenceTest ***************************
#************************************************************************
ADD_EXECUTABLE(vtkEncodeUncompressedSequenceTest vtkEncodeUncompressedSequenceTest.cxx)
SET_TARGET_PROPERTIES(vtkEncodeUncompressedSequenceTest PROPERTIES FOLDER Tests)
TARGET_LINK_LIBRARIES(vtkEncodeUncompressedSequenceTest vtkSlicerVideoIOModuleMRML )

#*************************** vtkMKVWriterTest1 ***************************
ADD_TEST(vtkEncodeUncompressedSequenceTest
  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/vtkEncodeUncompressedSequenceTest
    #--filename=mkvWriterTest1.mkv
    #--width=10
    #--height=10
    #--numFrames=100
    #--fps=10.0
  )
SET_TESTS_PROPERTIES(vtkEncodeUncompressedSequenceTest PROPERTIES FAIL_REGULAR_EXPRESSION "ERROR;WARNING")
