/*==============================================================================

Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
Queen's University, Kingston, ON, Canada. All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

This file was originally developed by Kyle Sunderland, PerkLab, Queen's University
and was supported through CANARIE's Research Software Program, and Cancer
Care Ontario.

==============================================================================*/

#include "vtkSlicerIGSIOLogger.h"

// IGSIO includes
#include <vtkIGSIOAccurateTimer.h>
#include <vtkIGSIORecursiveCriticalSection.h>

//-----------------------------------------------------------------------------
namespace
{
  vtkIGSIOSimpleRecursiveCriticalSection LoggerCreationCriticalSection;
}

//-------------------------------------------------------
vtkSlicerIGSIOLogger::vtkSlicerIGSIOLogger()
{
}

//-------------------------------------------------------
vtkSlicerIGSIOLogger::~vtkSlicerIGSIOLogger()
{
}

//-------------------------------------------------------
vtkIGSIOLogger* vtkSlicerIGSIOLogger::Instance()
{
  if (m_pInstance == NULL)
  {
    igsioLockGuard<vtkIGSIOSimpleRecursiveCriticalSection> loggerCreationGuard(&LoggerCreationCriticalSection);
    if (m_pInstance != NULL)
    {
      return m_pInstance;
    }

    vtkSlicerIGSIOLogger* newLoggerInstance = new vtkSlicerIGSIOLogger;
#ifdef VTK_HAS_INITIALIZE_OBJECT_BASE
    newLoggerInstance->InitializeObjectBase();
#endif

    // lock the instance even before making it available to make sure the instance is fully
    // initialized before anybody uses it
    igsioLockGuard<vtkIGSIORecursiveCriticalSection> critSectionGuard(newLoggerInstance->m_CriticalSection);
    m_pInstance = newLoggerInstance;
  }

  return m_pInstance;
}

//-------------------------------------------------------
void vtkSlicerIGSIOLogger::LogMessage(LogLevelType level, const char* msg, const char* fileName, int lineNumber, const char* optionalPrefix)
{
  if (m_LogLevel < level)
  {
    // no need to log
    return;
  }

  // If log level is not debug then only print messages for INFO logs (skip the INFO prefix, line numbers, etc.)
  bool onlyShowMessage = (level == LOG_LEVEL_INFO && m_LogLevel <= LOG_LEVEL_INFO);

  std::string timestamp = vtkIGSIOAccurateTimer::GetInstance()->GetDateAndTimeMSecString();

  std::ostringstream log;
  switch (level)
  {
  case LOG_LEVEL_ERROR:
    log << "ERROR: ";
    break;
  case LOG_LEVEL_WARNING:
    log << "WARNING: ";
    break;
  case LOG_LEVEL_INFO:
    log << "INFO: ";
    break;
  case LOG_LEVEL_DEBUG:
    log << "DEBUG: ";
    break;
  case LOG_LEVEL_TRACE:
    log << "TRACE: ";
    break;
  default:
    log << "UNKNOWN: ";
    break;
  }

  // Either pad out the log or add the optional prefix and pad
  if (optionalPrefix != NULL)
  {
    log << optionalPrefix << "> ";
  }
  else
  {
    log << " ";
  }

  log << msg;

  // Add the message to the log
  if (fileName != NULL)
  {
    log << " | in " << fileName << "(" << lineNumber << ")"; // add filename and line number
  }

  {
    igsioLockGuard<vtkIGSIORecursiveCriticalSection> critSectionGuard(this->m_CriticalSection);

    if (m_LogLevel >= level)
    {

#ifdef _WIN32

      // Set the text color to highlight error and warning messages (supported only on windows)
      switch (level)
      {
      case LOG_LEVEL_ERROR:
      {
        HANDLE hStdout = GetStdHandle(STD_ERROR_HANDLE);
        SetConsoleTextAttribute(hStdout, FOREGROUND_RED | FOREGROUND_INTENSITY);
      }
      break;
      case LOG_LEVEL_WARNING:
      {
        HANDLE hStdout = GetStdHandle(STD_ERROR_HANDLE);
        SetConsoleTextAttribute(hStdout, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
      }
      break;
      default:
      {
        HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleTextAttribute(hStdout, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
      }
      break;
      }
#endif

      if (level > LOG_LEVEL_WARNING)
      {
        if (onlyShowMessage)
        {
          std::cout << msg << std::endl;
        }
        else
        {
          std::cout << log.str() << std::endl;
        }
      }
      else
      {
        if (onlyShowMessage)
        {
          std::cerr << msg << std::endl;
        }
        else
        {
          std::cerr << log.str() << std::endl;
        }
      }

#ifdef _WIN32
      // Revert the text color (supported only on windows)
      if (level == LOG_LEVEL_ERROR || level == LOG_LEVEL_WARNING)
      {
        HANDLE hStdout = GetStdHandle(STD_ERROR_HANDLE);
        SetConsoleTextAttribute(hStdout, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
      }
#endif

      // Call display message callbacks if higher priority than trace
      if (level < LOG_LEVEL_TRACE)
      {
        std::ostringstream callDataStream;
        callDataStream << level << "|" << log.str();

        InvokeEvent(vtkSlicerIGSIOLogger::MessageLogged, (void*)(callDataStream.str().c_str()));
      }

      // Add to log stream (file)
      std::string logStr(log.str());
      std::wstring logWStr(logStr.begin(), logStr.end());
      this->m_LogStream << std::setw(17) << std::left << std::wstring(timestamp.begin(), timestamp.end()) << logWStr;
      this->m_LogStream << std::endl;
    }
  }

  this->Flush();
}

//----------------------------------------------------------------------------
void vtkSlicerIGSIOLogger::LogMessage(LogLevelType level, const wchar_t* msg, const char* fileName, int lineNumber, const wchar_t* optionalPrefix /*= NULL*/)
{
  if (m_LogLevel < level)
  {
    // no need to log
    return;
  }

  // If log level is not debug then only print messages for INFO logs (skip the INFO prefix, line numbers, etc.)
  bool onlyShowMessage = (level == LOG_LEVEL_INFO && m_LogLevel <= LOG_LEVEL_INFO);

  std::string timestamp = vtkIGSIOAccurateTimer::GetInstance()->GetDateAndTimeMSecString();

  std::wostringstream log;
  switch (level)
  {
  case LOG_LEVEL_ERROR:
    log << "ERROR: ";
    break;
  case LOG_LEVEL_WARNING:
    log << "WARNING: ";
    break;
  case LOG_LEVEL_INFO:
    log << "INFO: ";
    break;
  case LOG_LEVEL_DEBUG:
    log << "DEBUG: ";
    break;
  case LOG_LEVEL_TRACE:
    log << "TRACE: ";
    break;
  default:
    log << "UNKNOWN: ";
    break;
  }

  // Either pad out the log or add the optional prefix and pad
  if (optionalPrefix != NULL)
  {
    log << optionalPrefix << L"> ";
  }
  else
  {
    log << L" ";
  }

  log << msg;

  // Add the message to the log
  if (fileName != NULL)
  {
    log << L"| in " << fileName << L"(" << lineNumber << L")"; // add filename and line number
  }

  {
    igsioLockGuard<vtkIGSIORecursiveCriticalSection> critSectionGuard(this->m_CriticalSection);

    if (m_LogLevel >= level)
    {
#ifdef _WIN32
      // Set the text color to highlight error and warning messages (supported only on windows)
      switch (level)
      {
      case LOG_LEVEL_ERROR:
      {
        HANDLE hStdout = GetStdHandle(STD_ERROR_HANDLE);
        SetConsoleTextAttribute(hStdout, FOREGROUND_RED | FOREGROUND_INTENSITY);
      }
      break;
      case LOG_LEVEL_WARNING:
      {
        HANDLE hStdout = GetStdHandle(STD_ERROR_HANDLE);
        SetConsoleTextAttribute(hStdout, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
      }
      break;
      default:
      {
        HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleTextAttribute(hStdout, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
      }
      break;
      }
#endif

      if (level > LOG_LEVEL_WARNING)
      {
        if (onlyShowMessage)
        {
          std::wcout << msg << std::endl;
        }
        else
        {
          std::wcout << log.str() << std::endl;
        }
      }
      else
      {
        if (onlyShowMessage)
        {
          std::wcerr << msg << std::endl;
        }
        else
        {
          std::wcerr << log.str() << std::endl;
        }
      }

#ifdef _WIN32
      // Revert the text color (supported only on windows)
      if (level == LOG_LEVEL_ERROR || level == LOG_LEVEL_WARNING)
      {
        HANDLE hStdout = GetStdHandle(STD_ERROR_HANDLE);
        SetConsoleTextAttribute(hStdout, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
      }
#endif

      // Call display message callbacks if higher priority than trace
      if (level < LOG_LEVEL_TRACE)
      {
        std::wostringstream callDataStream;
        callDataStream << level << L"|" << log.str();

        InvokeEvent(vtkSlicerIGSIOLogger::WideMessageLogged, (void*)(callDataStream.str().c_str()));
      }

      // Add to log stream (file), this may introduce conversion issues going from wstring to string
      this->m_LogStream << std::setw(17) << std::left << std::wstring(timestamp.begin(), timestamp.end()) << log.str();
      this->m_LogStream << std::endl;
    }
  }

  this->Flush();
}
