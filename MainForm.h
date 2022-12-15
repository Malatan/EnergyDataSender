#pragma once
#define WIN32_LEAN_AND_MEAN
#include<windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <vcclr.h>
#include <wchar.h>
#include <msclr/marshal.h>
#include <msclr\marshal_cppstd.h>
#include <fstream>
#include <vector>
#include <string>

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

namespace EnergyDataSender {

	using namespace System::Runtime::InteropServices;
	using namespace System;
	using namespace System::Threading;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
	using namespace System::Collections::Generic;

	public ref class MainForm : public System::Windows::Forms::Form
	{
	public:
		MainForm(void)
		{
			System::Windows::Forms::Control::CheckForIllegalCrossThreadCalls = false;
			InitializeComponent();
			this->records = gcnew List<String^>();
			
		}

	protected:
		~MainForm()
		{
			if (components)
			{
				delete components;
			}
		}

	protected:
	private: System::Int32 timer = 10;
	private: System::Int32 step = 400;
	private: System::Int32 countDown = 10;
	private: System::Int32 recordsLoaded = 0;
	private: System::Int32 recordsSent = 0;
	private: System::Boolean endThread = false;
	private: System::Boolean start = false;
	private: System::Boolean pauseTimerWorker = false;
	private: List<String^>^ records;
	private: System::Windows::Forms::Label^ recordsCountlbl;
	private: System::Windows::Forms::Label^ recordsCountValuelbl;
	private: System::Windows::Forms::Label^ recordsSentlbl;
	private: System::Windows::Forms::Label^ recordsLeftlbl;
	private: System::Windows::Forms::ProgressBar^ countdownBar;
	private: System::Windows::Forms::Label^ statusMsglbl;
	private: System::Windows::Forms::Button^ startBtn;
	private: System::Windows::Forms::Button^ setTimerBtn;
	private: System::Windows::Forms::Label^ timerlbl;
	private: System::Windows::Forms::Label^ recordsSentValuelbl;
	private: System::Windows::Forms::Label^ recordsLeftValuelbl;
	private: System::Windows::Forms::Label^ timerValuelbl;
	private: System::Windows::Forms::Button^ loadCSVBtn;
	private: System::Windows::Forms::Button^ resetRecordsBtn;
	private: System::Windows::Forms::TextBox^ timerTextBox;
	private: System::ComponentModel::BackgroundWorker^ timerWorker;
	private: System::Windows::Forms::TextBox^ host1TextBox;
	private: System::Windows::Forms::TextBox^ host2TextBox;
	private: System::Windows::Forms::TextBox^ host3TextBox;
	private: System::Windows::Forms::TextBox^ host4TextBox;
	private: System::Windows::Forms::TextBox^ host5TextBox;
	private: System::Windows::Forms::OpenFileDialog^ openFileDialog1;




	private: System::Windows::Forms::TextBox^ stepTextBox;
	private: System::Windows::Forms::Label^ stepValuelbl;
	private: System::Windows::Forms::Label^ steplbl;
	private: System::Windows::Forms::Button^ setStepBtn;
	private: System::Windows::Forms::Label^ errorMsglbl;
	private:System::ComponentModel::Container^ components;

		   Boolean sendData() {
			   String^ host_address = this->host1TextBox->Text + "." + this->host2TextBox->Text + "." +
				   this->host3TextBox->Text + "." + this->host4TextBox->Text;
			   int portno = Convert::ToInt32(this->host5TextBox->Text);
			   this->statusMsglbl->Text = "Connecting to " + host_address + ":" + portno;

			   System::Diagnostics::Debug::WriteLine("Sending " + this->step + " record");

			   String^ send_msg;
			   int records_to_send = this->step;
			   Boolean clear_records = false;
			   if (this->records->Count < records_to_send) {
				   records_to_send = this->records->Count;
				   clear_records = true;
			   }
			   for (int i = 0; i < records_to_send; i++) {
				   send_msg += this->records[i] + ";";
			   }
			   
			   System::Diagnostics::Debug::WriteLine(send_msg->Length);
			   std::string unmanaged_send_msg = msclr::interop::marshal_as<std::string>(send_msg);
			   WSADATA wsaData;
			   SOCKET ConnectSocket = INVALID_SOCKET;
			   struct addrinfo* result = NULL,
				   * ptr = NULL,
				   hints;
			   const char* sendbuf = unmanaged_send_msg.c_str();
			   int iResult;

			   // Initialize Winsock
			   iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
			   if (iResult != 0) {
				   this->errorMsglbl->Text = "WSAStartup failed with error";
				   System::Diagnostics::Debug::WriteLine("WSAStartup failed with error");
				   return false;
			   }

			   ZeroMemory(&hints, sizeof(hints));
			   hints.ai_family = AF_UNSPEC;
			   hints.ai_socktype = SOCK_STREAM;
			   hints.ai_protocol = IPPROTO_TCP;

			   std::string unmanaged_host_address = msclr::interop::marshal_as<std::string>(host_address);
			   std::string unmanaged_portno = msclr::interop::marshal_as<std::string>(Convert::ToString(portno));

			   // Resolve the server address and port
			   this->statusMsglbl->Text = "Resolving the address...";
			   System::Diagnostics::Debug::WriteLine("Resolving the address...");
			   iResult = getaddrinfo(unmanaged_host_address.c_str(), unmanaged_portno.c_str(), &hints, &result);
			   if (iResult != 0) {
				   this->errorMsglbl->Text = "getaddrinfo failed with error";
				   System::Diagnostics::Debug::WriteLine("getaddrinfo failed with error");
				   WSACleanup();
				   return false;
			   }

			   // Attempt to connect to an address until one succeeds
			   this->statusMsglbl->Text = "Connecting to server...";
			   System::Diagnostics::Debug::WriteLine("Connecting to server...");
			   for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
				   // Create a SOCKET for connecting to server
				   ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
					   ptr->ai_protocol);
				   if (ConnectSocket == INVALID_SOCKET) {
					   this->errorMsglbl->Text = "socket failed with error";
					   System::Diagnostics::Debug::WriteLine("socket failed with error");
					   WSACleanup();
					   return false;
				   }
				   // Connect to server.
				   iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
				   if (iResult == SOCKET_ERROR) {
					   closesocket(ConnectSocket);
					   ConnectSocket = INVALID_SOCKET;
					   System::Diagnostics::Debug::WriteLine("socket error");
					   continue;
				   }
				   break;
			   }

			   freeaddrinfo(result);

			   if (ConnectSocket == INVALID_SOCKET) {
				   this->errorMsglbl->Text = "Unable to connect to server!";
				   System::Diagnostics::Debug::WriteLine("Unable to connect to server!");
				   WSACleanup();
				   return false;
			   }

			   this->statusMsglbl->Text = "Sending data...";
			   System::Diagnostics::Debug::WriteLine("Sending data...");
			   // Send an initial buffer
			   iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
			   if (iResult == SOCKET_ERROR) {
				   this->errorMsglbl->Text = "send failed with error";
				   closesocket(ConnectSocket);
				   WSACleanup();
				   return false;
			   }

			   this->statusMsglbl->Text = "Closing connecting...";
			   System::Diagnostics::Debug::WriteLine("Closing connecting...");
			   // shutdown the connection since no more data will be sent
			   iResult = shutdown(ConnectSocket, SD_SEND);
			   if (iResult == SOCKET_ERROR) {
				   this->errorMsglbl->Text = "shutdown failed with error";
				   closesocket(ConnectSocket);
				   WSACleanup();
				   return false;
			   }


			   closesocket(ConnectSocket);
			   WSACleanup();
			   if (clear_records) {
				   this->records = gcnew List<String^>();
			   }
			   else {
				   this->records->RemoveRange(0, records_to_send);
			   }
			   this->recordsSent += records_to_send;
			   System::Diagnostics::Debug::WriteLine("End tcp");
			   return true;
		   }

		   void updateValueLbls() {
			   this->recordsCountValuelbl->Text = this->recordsLoaded + "";
			   this->recordsLeftValuelbl->Text = this->records->Count + "";
			   this->recordsSentValuelbl->Text = this->recordsSent + "";
			   this->timerValuelbl->Text = this->timer + " seconds";
			   this->stepValuelbl->Text = this->step + "";
		   }

#pragma region Windows Form Designer generated code
		   /// <summary>
		   /// Required method for Designer support - do not modify
		   /// the contents of this method with the code editor.
		   /// </summary>
		   void InitializeComponent(void)
		   {
			   this->recordsCountlbl = (gcnew System::Windows::Forms::Label());
			   this->recordsCountValuelbl = (gcnew System::Windows::Forms::Label());
			   this->recordsSentlbl = (gcnew System::Windows::Forms::Label());
			   this->recordsLeftlbl = (gcnew System::Windows::Forms::Label());
			   this->countdownBar = (gcnew System::Windows::Forms::ProgressBar());
			   this->statusMsglbl = (gcnew System::Windows::Forms::Label());
			   this->startBtn = (gcnew System::Windows::Forms::Button());
			   this->setTimerBtn = (gcnew System::Windows::Forms::Button());
			   this->timerlbl = (gcnew System::Windows::Forms::Label());
			   this->recordsSentValuelbl = (gcnew System::Windows::Forms::Label());
			   this->recordsLeftValuelbl = (gcnew System::Windows::Forms::Label());
			   this->timerValuelbl = (gcnew System::Windows::Forms::Label());
			   this->loadCSVBtn = (gcnew System::Windows::Forms::Button());
			   this->resetRecordsBtn = (gcnew System::Windows::Forms::Button());
			   this->timerTextBox = (gcnew System::Windows::Forms::TextBox());
			   this->timerWorker = (gcnew System::ComponentModel::BackgroundWorker());
			   this->host1TextBox = (gcnew System::Windows::Forms::TextBox());
			   this->host2TextBox = (gcnew System::Windows::Forms::TextBox());
			   this->host3TextBox = (gcnew System::Windows::Forms::TextBox());
			   this->host4TextBox = (gcnew System::Windows::Forms::TextBox());
			   this->host5TextBox = (gcnew System::Windows::Forms::TextBox());
			   this->openFileDialog1 = (gcnew System::Windows::Forms::OpenFileDialog());
			   this->stepTextBox = (gcnew System::Windows::Forms::TextBox());
			   this->stepValuelbl = (gcnew System::Windows::Forms::Label());
			   this->steplbl = (gcnew System::Windows::Forms::Label());
			   this->setStepBtn = (gcnew System::Windows::Forms::Button());
			   this->errorMsglbl = (gcnew System::Windows::Forms::Label());
			   this->SuspendLayout();
			   // 
			   // recordsCountlbl
			   // 
			   this->recordsCountlbl->AutoSize = true;
			   this->recordsCountlbl->Location = System::Drawing::Point(12, 10);
			   this->recordsCountlbl->Name = L"recordsCountlbl";
			   this->recordsCountlbl->Size = System::Drawing::Size(95, 12);
			   this->recordsCountlbl->TabIndex = 0;
			   this->recordsCountlbl->Text = L"Records loaded:";
			   // 
			   // recordsCountValuelbl
			   // 
			   this->recordsCountValuelbl->AutoSize = true;
			   this->recordsCountValuelbl->Location = System::Drawing::Point(113, 9);
			   this->recordsCountValuelbl->Name = L"recordsCountValuelbl";
			   this->recordsCountValuelbl->Size = System::Drawing::Size(11, 12);
			   this->recordsCountValuelbl->TabIndex = 1;
			   this->recordsCountValuelbl->Text = L"0";
			   // 
			   // recordsSentlbl
			   // 
			   this->recordsSentlbl->AutoSize = true;
			   this->recordsSentlbl->Location = System::Drawing::Point(12, 30);
			   this->recordsSentlbl->Name = L"recordsSentlbl";
			   this->recordsSentlbl->Size = System::Drawing::Size(83, 12);
			   this->recordsSentlbl->TabIndex = 2;
			   this->recordsSentlbl->Text = L"Records sent:";
			   // 
			   // recordsLeftlbl
			   // 
			   this->recordsLeftlbl->AutoSize = true;
			   this->recordsLeftlbl->Location = System::Drawing::Point(12, 50);
			   this->recordsLeftlbl->Name = L"recordsLeftlbl";
			   this->recordsLeftlbl->Size = System::Drawing::Size(83, 12);
			   this->recordsLeftlbl->TabIndex = 3;
			   this->recordsLeftlbl->Text = L"Records left:";
			   // 
			   // countdownBar
			   // 
			   this->countdownBar->Location = System::Drawing::Point(14, 246);
			   this->countdownBar->Maximum = 10;
			   this->countdownBar->Name = L"countdownBar";
			   this->countdownBar->Size = System::Drawing::Size(260, 23);
			   this->countdownBar->Step = 1;
			   this->countdownBar->TabIndex = 4;
			   // 
			   // statusMsglbl
			   // 
			   this->statusMsglbl->AutoSize = true;
			   this->statusMsglbl->Location = System::Drawing::Point(12, 272);
			   this->statusMsglbl->Name = L"statusMsglbl";
			   this->statusMsglbl->Size = System::Drawing::Size(47, 12);
			   this->statusMsglbl->TabIndex = 5;
			   this->statusMsglbl->Text = L"Idle...";
			   // 
			   // startBtn
			   // 
			   this->startBtn->Location = System::Drawing::Point(14, 183);
			   this->startBtn->Name = L"startBtn";
			   this->startBtn->Size = System::Drawing::Size(260, 30);
			   this->startBtn->TabIndex = 6;
			   this->startBtn->Text = L"Start";
			   this->startBtn->UseVisualStyleBackColor = true;
			   this->startBtn->Click += gcnew System::EventHandler(this, &MainForm::startBtn_Click);
			   // 
			   // setTimerBtn
			   // 
			   this->setTimerBtn->Location = System::Drawing::Point(188, 79);
			   this->setTimerBtn->Name = L"setTimerBtn";
			   this->setTimerBtn->Size = System::Drawing::Size(86, 30);
			   this->setTimerBtn->TabIndex = 7;
			   this->setTimerBtn->Text = L"Change";
			   this->setTimerBtn->UseVisualStyleBackColor = true;
			   this->setTimerBtn->Click += gcnew System::EventHandler(this, &MainForm::setTimerlbl_Click);
			   // 
			   // timerlbl
			   // 
			   this->timerlbl->AutoSize = true;
			   this->timerlbl->Location = System::Drawing::Point(12, 70);
			   this->timerlbl->Name = L"timerlbl";
			   this->timerlbl->Size = System::Drawing::Size(41, 12);
			   this->timerlbl->TabIndex = 8;
			   this->timerlbl->Text = L"Timer:";
			   // 
			   // recordsSentValuelbl
			   // 
			   this->recordsSentValuelbl->AutoSize = true;
			   this->recordsSentValuelbl->Location = System::Drawing::Point(101, 30);
			   this->recordsSentValuelbl->Name = L"recordsSentValuelbl";
			   this->recordsSentValuelbl->Size = System::Drawing::Size(11, 12);
			   this->recordsSentValuelbl->TabIndex = 10;
			   this->recordsSentValuelbl->Text = L"0";
			   // 
			   // recordsLeftValuelbl
			   // 
			   this->recordsLeftValuelbl->AutoSize = true;
			   this->recordsLeftValuelbl->Location = System::Drawing::Point(101, 50);
			   this->recordsLeftValuelbl->Name = L"recordsLeftValuelbl";
			   this->recordsLeftValuelbl->Size = System::Drawing::Size(11, 12);
			   this->recordsLeftValuelbl->TabIndex = 11;
			   this->recordsLeftValuelbl->Text = L"0";
			   // 
			   // timerValuelbl
			   // 
			   this->timerValuelbl->AutoSize = true;
			   this->timerValuelbl->Location = System::Drawing::Point(59, 70);
			   this->timerValuelbl->Name = L"timerValuelbl";
			   this->timerValuelbl->Size = System::Drawing::Size(65, 12);
			   this->timerValuelbl->TabIndex = 12;
			   this->timerValuelbl->Text = L"10 seconds";
			   // 
			   // loadCSVBtn
			   // 
			   this->loadCSVBtn->Location = System::Drawing::Point(14, 154);
			   this->loadCSVBtn->Name = L"loadCSVBtn";
			   this->loadCSVBtn->Size = System::Drawing::Size(120, 23);
			   this->loadCSVBtn->TabIndex = 13;
			   this->loadCSVBtn->Text = L"Load CSV";
			   this->loadCSVBtn->UseVisualStyleBackColor = true;
			   this->loadCSVBtn->Click += gcnew System::EventHandler(this, &MainForm::loadCSVBtn_Click);
			   // 
			   // resetRecordsBtn
			   // 
			   this->resetRecordsBtn->Location = System::Drawing::Point(154, 154);
			   this->resetRecordsBtn->Name = L"resetRecordsBtn";
			   this->resetRecordsBtn->Size = System::Drawing::Size(120, 23);
			   this->resetRecordsBtn->TabIndex = 15;
			   this->resetRecordsBtn->Text = L"Reset Records";
			   this->resetRecordsBtn->UseVisualStyleBackColor = true;
			   this->resetRecordsBtn->Click += gcnew System::EventHandler(this, &MainForm::resetRecordsBtn_Click);
			   // 
			   // timerTextBox
			   // 
			   this->timerTextBox->Location = System::Drawing::Point(14, 85);
			   this->timerTextBox->Name = L"timerTextBox";
			   this->timerTextBox->Size = System::Drawing::Size(168, 21);
			   this->timerTextBox->TabIndex = 20;
			   this->timerTextBox->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &MainForm::timerTextBox_KeyPress);
			   // 
			   // timerWorker
			   // 
			   this->timerWorker->WorkerReportsProgress = true;
			   this->timerWorker->WorkerSupportsCancellation = true;
			   this->timerWorker->DoWork += gcnew System::ComponentModel::DoWorkEventHandler(this, &MainForm::timerWorker_DoWork);
			   this->timerWorker->ProgressChanged += gcnew System::ComponentModel::ProgressChangedEventHandler(this, &MainForm::timerWorker_ProgressChanged);
			   this->timerWorker->RunWorkerCompleted += gcnew System::ComponentModel::RunWorkerCompletedEventHandler(this, &MainForm::timerWorker_RunWorkerCompleted);
			   // 
			   // host1TextBox
			   // 
			   this->host1TextBox->Location = System::Drawing::Point(16, 219);
			   this->host1TextBox->MaxLength = 3;
			   this->host1TextBox->Name = L"host1TextBox";
			   this->host1TextBox->Size = System::Drawing::Size(40, 21);
			   this->host1TextBox->TabIndex = 22;
			   this->host1TextBox->Text = L"127";
			   this->host1TextBox->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &MainForm::host1TextBox_KeyPress);
			   // 
			   // host2TextBox
			   // 
			   this->host2TextBox->Location = System::Drawing::Point(62, 219);
			   this->host2TextBox->MaxLength = 3;
			   this->host2TextBox->Name = L"host2TextBox";
			   this->host2TextBox->Size = System::Drawing::Size(40, 21);
			   this->host2TextBox->TabIndex = 23;
			   this->host2TextBox->Text = L"0";
			   this->host2TextBox->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &MainForm::host2TextBox_KeyPress);
			   // 
			   // host3TextBox
			   // 
			   this->host3TextBox->Location = System::Drawing::Point(108, 219);
			   this->host3TextBox->MaxLength = 3;
			   this->host3TextBox->Name = L"host3TextBox";
			   this->host3TextBox->Size = System::Drawing::Size(40, 21);
			   this->host3TextBox->TabIndex = 24;
			   this->host3TextBox->Text = L"0";
			   this->host3TextBox->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &MainForm::host3TextBox_KeyPress);
			   // 
			   // host4TextBox
			   // 
			   this->host4TextBox->Location = System::Drawing::Point(154, 219);
			   this->host4TextBox->MaxLength = 3;
			   this->host4TextBox->Name = L"host4TextBox";
			   this->host4TextBox->Size = System::Drawing::Size(40, 21);
			   this->host4TextBox->TabIndex = 25;
			   this->host4TextBox->Text = L"1";
			   this->host4TextBox->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &MainForm::host4TextBox_KeyPress);
			   // 
			   // host5TextBox
			   // 
			   this->host5TextBox->Location = System::Drawing::Point(213, 219);
			   this->host5TextBox->MaxLength = 5;
			   this->host5TextBox->Name = L"host5TextBox";
			   this->host5TextBox->Size = System::Drawing::Size(61, 21);
			   this->host5TextBox->TabIndex = 26;
			   this->host5TextBox->Text = L"8889";
			   this->host5TextBox->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &MainForm::host5TextBox_KeyPress);
			   // 
			   // openFileDialog1
			   // 
			   this->openFileDialog1->FileName = L"openFileDialog1";
			   this->openFileDialog1->Filter = L"csv files (*.csv)|*.csv";
			   this->openFileDialog1->InitialDirectory = L"c:\\";
			   this->openFileDialog1->RestoreDirectory = true;
			   this->openFileDialog1->ShowHelp = true;
			   // 
			   // stepTextBox
			   // 
			   this->stepTextBox->Location = System::Drawing::Point(14, 124);
			   this->stepTextBox->Name = L"stepTextBox";
			   this->stepTextBox->Size = System::Drawing::Size(168, 21);
			   this->stepTextBox->TabIndex = 40;
			   this->stepTextBox->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &MainForm::stepTextBox_KeyPress);
			   // 
			   // stepValuelbl
			   // 
			   this->stepValuelbl->AutoSize = true;
			   this->stepValuelbl->Location = System::Drawing::Point(53, 109);
			   this->stepValuelbl->Name = L"stepValuelbl";
			   this->stepValuelbl->Size = System::Drawing::Size(23, 12);
			   this->stepValuelbl->TabIndex = 39;
			   this->stepValuelbl->Text = L"400";
			   // 
			   // steplbl
			   // 
			   this->steplbl->AutoSize = true;
			   this->steplbl->Location = System::Drawing::Point(12, 109);
			   this->steplbl->Name = L"steplbl";
			   this->steplbl->Size = System::Drawing::Size(35, 12);
			   this->steplbl->TabIndex = 38;
			   this->steplbl->Text = L"Step:";
			   // 
			   // setStepBtn
			   // 
			   this->setStepBtn->Location = System::Drawing::Point(188, 118);
			   this->setStepBtn->Name = L"setStepBtn";
			   this->setStepBtn->Size = System::Drawing::Size(86, 30);
			   this->setStepBtn->TabIndex = 37;
			   this->setStepBtn->Text = L"Change";
			   this->setStepBtn->UseVisualStyleBackColor = true;
			   this->setStepBtn->Click += gcnew System::EventHandler(this, &MainForm::setStepBtn_Click);
			   // 
			   // errorMsglbl
			   // 
			   this->errorMsglbl->AutoSize = true;
			   this->errorMsglbl->Location = System::Drawing::Point(12, 285);
			   this->errorMsglbl->Name = L"errorMsglbl";
			   this->errorMsglbl->Size = System::Drawing::Size(0, 12);
			   this->errorMsglbl->TabIndex = 41;
			   // 
			   // MainForm
			   // 
			   this->AutoScaleDimensions = System::Drawing::SizeF(6, 12);
			   this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			   this->ClientSize = System::Drawing::Size(284, 291);
			   this->Controls->Add(this->errorMsglbl);
			   this->Controls->Add(this->stepTextBox);
			   this->Controls->Add(this->stepValuelbl);
			   this->Controls->Add(this->steplbl);
			   this->Controls->Add(this->setStepBtn);
			   this->Controls->Add(this->host5TextBox);
			   this->Controls->Add(this->host4TextBox);
			   this->Controls->Add(this->host3TextBox);
			   this->Controls->Add(this->host2TextBox);
			   this->Controls->Add(this->host1TextBox);
			   this->Controls->Add(this->timerTextBox);
			   this->Controls->Add(this->resetRecordsBtn);
			   this->Controls->Add(this->loadCSVBtn);
			   this->Controls->Add(this->timerValuelbl);
			   this->Controls->Add(this->recordsLeftValuelbl);
			   this->Controls->Add(this->recordsSentValuelbl);
			   this->Controls->Add(this->timerlbl);
			   this->Controls->Add(this->setTimerBtn);
			   this->Controls->Add(this->startBtn);
			   this->Controls->Add(this->statusMsglbl);
			   this->Controls->Add(this->countdownBar);
			   this->Controls->Add(this->recordsLeftlbl);
			   this->Controls->Add(this->recordsSentlbl);
			   this->Controls->Add(this->recordsCountValuelbl);
			   this->Controls->Add(this->recordsCountlbl);
			   this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedToolWindow;
			   this->Name = L"MainForm";
			   this->StartPosition = System::Windows::Forms::FormStartPosition::CenterScreen;
			   this->Text = L"EnergyDataSender";
			   this->Load += gcnew System::EventHandler(this, &MainForm::MyForm_Load);
			   this->ResumeLayout(false);
			   this->PerformLayout();

		   }
#pragma endregion
	private: System::Void MyForm_Load(System::Object^ sender, System::EventArgs^ e) {
		MessageBox::Show("Made by Zheng\nFor university project propose", "Author");
	}
	private: System::Void timerTextBox_KeyPress(System::Object^ sender, System::Windows::Forms::KeyPressEventArgs^ e) {
		if (!Char::IsControl(e->KeyChar) && !Char::IsDigit(e->KeyChar)) {
			e->Handled = true;
		}
	}
	private: System::Void nSourceTextBox_KeyPress(System::Object^ sender, System::Windows::Forms::KeyPressEventArgs^ e) {
		if (!Char::IsControl(e->KeyChar) && !Char::IsDigit(e->KeyChar)) {
			e->Handled = true;
		}
	}
	private: System::Void stepTextBox_KeyPress(System::Object^ sender, System::Windows::Forms::KeyPressEventArgs^ e) {
		if (!Char::IsControl(e->KeyChar) && !Char::IsDigit(e->KeyChar)) {
			e->Handled = true;
		}
	}
	private: System::Void setTimerlbl_Click(System::Object^ sender, System::EventArgs^ e) {
		if (this->timerTextBox->Text != "") {
			int new_timer = Convert::ToInt32(this->timerTextBox->Text);
			this->timer = new_timer;

			this->countdownBar->Value = 0;
			this->countdownBar->Maximum = new_timer;

			this->updateValueLbls();
			this->timerTextBox->Text = "";
			MessageBox::Show("Set the timer to " + new_timer, "Message");
		}
		else {
			MessageBox::Show("Invalid input", "Message");
		}
	}
	private: System::Void setStepBtn_Click(System::Object^ sender, System::EventArgs^ e) {
		if (this->stepTextBox->Text != "") {
			this->step = Convert::ToInt32(this->stepTextBox->Text);
			this->stepValuelbl->Text = this->step + "";
			this->stepTextBox->Text = "";
			MessageBox::Show("Set the step to " + this->step, "Message");
		}
		else {
			MessageBox::Show("Invalid input", "Message");
		}
	}
	private: System::Void startBtn_Click(System::Object^ sender, System::EventArgs^ e) {
		if (this->start) {
			this->start = false;
			this->startBtn->Text = "Start";
			this->host1TextBox->Enabled = true;
			this->host2TextBox->Enabled = true;
			this->host3TextBox->Enabled = true;
			this->host4TextBox->Enabled = true;
			this->host5TextBox->Enabled = true;
			this->timerWorker->CancelAsync();
		}
		else {
			if ((this->host1TextBox->TextLength == 0 || this->host2TextBox->TextLength == 0 ||
				this->host3TextBox->TextLength == 0 || this->host4TextBox->TextLength == 0 ||
				this->host5TextBox->TextLength == 0)) {
				this->statusMsglbl->Text = "Invalid host address";
			}
			else {
				this->start = true;
				this->startBtn->Text = "Stop";
				this->host1TextBox->Enabled = false;
				this->host2TextBox->Enabled = false;
				this->host3TextBox->Enabled = false;
				this->host4TextBox->Enabled = false;
				this->host5TextBox->Enabled = false;
				this->timerWorker->RunWorkerAsync();
			}
		}
	}
	private: System::Void host1TextBox_KeyPress(System::Object^ sender, System::Windows::Forms::KeyPressEventArgs^ e) {
		if (!Char::IsControl(e->KeyChar) && !Char::IsDigit(e->KeyChar)) {
			e->Handled = true;
		}
	}
	private: System::Void host2TextBox_KeyPress(System::Object^ sender, System::Windows::Forms::KeyPressEventArgs^ e) {
		if (!Char::IsControl(e->KeyChar) && !Char::IsDigit(e->KeyChar)) {
			e->Handled = true;
		}
	}
	private: System::Void host3TextBox_KeyPress(System::Object^ sender, System::Windows::Forms::KeyPressEventArgs^ e) {
		if (!Char::IsControl(e->KeyChar) && !Char::IsDigit(e->KeyChar)) {
			e->Handled = true;
		}
	}
	private: System::Void host4TextBox_KeyPress(System::Object^ sender, System::Windows::Forms::KeyPressEventArgs^ e) {
		if (!Char::IsControl(e->KeyChar) && !Char::IsDigit(e->KeyChar)) {
			e->Handled = true;
		}
	}
	private: System::Void host5TextBox_KeyPress(System::Object^ sender, System::Windows::Forms::KeyPressEventArgs^ e) {
		if (!Char::IsControl(e->KeyChar) && !Char::IsDigit(e->KeyChar)) {
			e->Handled = true;
		}
	}
	private: System::Void timerWorker_DoWork(System::Object^ sender, System::ComponentModel::DoWorkEventArgs^ e) {
		System::Diagnostics::Debug::WriteLine("Timer is working");
		this->countDown = this->timer;
		while (this->start) {
			if (this->timerWorker->CancellationPending) {
				e->Cancel = true;
				System::Diagnostics::Debug::WriteLine("Timer is cancelled");
				break;
			}
			this->timerWorker->ReportProgress(1);
			Thread::Sleep(1000);
		}
	}
	private: System::Void timerWorker_ProgressChanged(System::Object^ sender, System::ComponentModel::ProgressChangedEventArgs^ e) {
		if (this->countdownBar->Value + 1 > this->countdownBar->Maximum) {
			System::Diagnostics::Debug::WriteLine("Timer is up");
			System::Diagnostics::Debug::WriteLine("Start send data worker");
			this->errorMsglbl->Text = "";
			this->countdownBar->Value = 0;
			this->countDown = this->timer;
			if (this->records->Count > 0) {
				System::Diagnostics::Debug::WriteLine("Sending " + this->step + " records");
				this->sendData();
				this->updateValueLbls();
			}
			else {
				System::Diagnostics::Debug::WriteLine("No records to be sent");
			}
		}
		this->countdownBar->Value++;
		this->countDown--;
		this->statusMsglbl->Text = "Next send in " + this->countDown + "s";
		
	}
	private: System::Void timerWorker_RunWorkerCompleted(System::Object^ sender, System::ComponentModel::RunWorkerCompletedEventArgs^ e) {
		System::Diagnostics::Debug::WriteLine("Timer stopped(finished)");
		this->countdownBar->Value = 0;
		this->statusMsglbl->Text = "Stopped";
	}
	private: System::Void loadCSVBtn_Click(System::Object^ sender, System::EventArgs^ e) {
		if (openFileDialog1->ShowDialog() == System::Windows::Forms::DialogResult::OK) {
			String^ file_path = openFileDialog1->FileName;
			String^ file_name = System::IO::Path::GetFileName(file_path);
			this->statusMsglbl->Text = "Loading " + file_name;
			std::string unmanaged_file_path = msclr::interop::marshal_as<std::string>(openFileDialog1->FileName);
			std::ifstream csv_data(unmanaged_file_path, std::ios::in);
			std::string line;
			std::getline(csv_data, line);
			int line_counter = 0;
			while (std::getline(csv_data, line)) {
				String^ m_line = gcnew String(line.c_str());
				this->records->Add(m_line);
				line_counter++;
			}
			System::Diagnostics::Debug::WriteLine(this->records->Count);
			this->statusMsglbl->Text = this->records->Count + " records loaded";
			this->recordsLoaded += line_counter;
			this->updateValueLbls();
			csv_data.close();
		}
	}
	private: System::Void resetRecordsBtn_Click(System::Object^ sender, System::EventArgs^ e) {
		this->records = gcnew List<String^>();
		this->updateValueLbls();
		MessageBox::Show("Cleared all records", "Message");
		System::Diagnostics::Debug::WriteLine("Records count: " + this->records->Count);
	}
};
}
